#include "assets.h"
#include "logger.h"
#include "assetloader.h"
#include <fstream>
#include <filesystem>
#include <cstring>
#include "logger.h"

namespace assets {
TextureManager *TextureManager::get() {
  static TextureManager *texturemanager = new TextureManager;
  return texturemanager;
}

void TextureManager::bindTexture(const std::string &name, GLenum texturei) {
  if (!textures.count(name))
    return;
  TextureInfo info = textures.at(name);
  glActiveTexture(texturei);
  glBindTexture(info.target, info.id);
}

TextureMetaData entryToTextureMetaData(const impfile::Entry &entry) {
  TextureMetaData texture;

  texture.name = entry.name;
  texture.target = entry.getVar("target");

  if (texture.target == "cubemap") {
    texture.cubemapPaths = {
        entry.getVar("east"), entry.getVar("west"),  entry.getVar("up"),
        entry.getVar("down"), entry.getVar("north"), entry.getVar("south"),
    };
  }
  // Preferably target is 'texture2d' but it defaults to texture2d
  else {
    texture.path = entry.getVar("path");
    std::string flip = entry.getVar("flip");
    if (flip == "true")
      texture.flipv = true;
    // Preferably flip is "false" but if it is an unrecognized value,
    // it will default to false
    else
      texture.flipv = false;
  }

  return texture;
}

TextureInfo textureMetaDataToInfo(const TextureMetaData &metadata,
                                  unsigned int id) {
  TextureInfo info;
  info.id = id;

  if (metadata.target == "cubemap") {
    info.target = GL_TEXTURE_CUBE_MAP;
    gfx::loadCubemap(metadata.cubemapPaths, info.id);
  } else {
    info.target = GL_TEXTURE_2D;
    gfx::loadTexture(metadata.path.c_str(), info.id, metadata.flipv);
  }

  return info;
}

void TextureManager::importFromFile(const char *path) {
  AssetLoader& loader = AssetLoader::getInstance();

  // Load .impfile from packed assets
  std::string impfileContent = loader.getAssetString(path);
  if (impfileContent.empty()) {
    ERROR("Failed to load texture config: %s", path);
    return;
  }

  // Parse in memory
  std::vector<impfile::Entry> entries = impfile::parseBin(impfileContent);

  std::vector<unsigned int> textureids(entries.size());
  glGenTextures(entries.size(), &textureids[0]);

  for (size_t i = 0; i < entries.size(); i++) {
    const impfile::Entry &entry = entries.at(i);
    TextureMetaData metadata = entryToTextureMetaData(entry);
    unsigned id = textureids.at(i);
    TextureInfo texinfo = textureMetaDataToInfo(metadata, id);
    textures.insert({entry.name, texinfo});
  }
}

ShaderMetaData entryToShaderMetaData(const impfile::Entry &entry) {
  ShaderMetaData metadata;

  metadata.name = entry.name;
  metadata.vertpath = entry.getVar("vertex");
  metadata.fragpath = entry.getVar("fragment");

  return metadata;
}

void ShaderManager::importFromFile(const char *path) {
  AssetLoader& loader = AssetLoader::getInstance();

  // Load .impfile from packed assets
  std::string impfileContent = loader.getAssetString(path);
  if (impfileContent.empty()) {
    ERROR("Failed to load shader config: %s", path);
    return;
  }

  // Parse in memory using parseBin
  std::vector<impfile::Entry> entries = impfile::parseBin(impfileContent);
  
  for (size_t i = 0; i < entries.size(); i++) {
    const impfile::Entry &entry = entries.at(i);
    ShaderMetaData metadata = entryToShaderMetaData(entry);

    // Load shader source from packed assets
    std::string vertSource = loader.getAssetString(metadata.vertpath.c_str());
    std::string fragSource = loader.getAssetString(metadata.fragpath.c_str());

    if (vertSource.empty() || fragSource.empty()) {
      fprintf(stderr, "Failed to load shader sources for: %s\n", metadata.name.c_str());
      continue;
    }

    // Write to temp files (shader compiler needs file paths)
    // Use cross-platform temp directory (works on Windows, macOS, Linux, Android)
    std::filesystem::path tempDir = std::filesystem::temp_directory_path();
    std::string tempVertPath = (tempDir / (metadata.name + "_vert.glsl")).string();
    std::string tempFragPath = (tempDir / (metadata.name + "_frag.glsl")).string();

    std::ofstream vertFile(tempVertPath);
    vertFile << vertSource;
    vertFile.close();

    std::ofstream fragFile(tempFragPath);
    fragFile << fragSource;
    fragFile.close();

    // Create shader program
    ShaderProgram program(tempVertPath.c_str(), tempFragPath.c_str());
    shaders.insert({metadata.name, program});

    // Cleanup temp files
    std::remove(tempVertPath.c_str());
    std::remove(tempFragPath.c_str());
  }
}

void ShaderManager::use(const std::string &name) {
  if (!shaders.count(name))
    return;
  shaders.at(name).use();
}

ShaderProgram &ShaderManager::getShader(const std::string &name) {
  if (!shaders.count(name)) {
    ERROR("%s does not exist as a shader!", name.c_str());
    exit(1); // Crash the program, this hopefully shouldn't happen
  }
  return static_cast<ShaderProgram &>(shaders.at(name));
}

ShaderManager *ShaderManager::get() {
  static ShaderManager *shadermanager = new ShaderManager;
  return shadermanager;
}

ModelMetaData entryToModelMetaData(const impfile::Entry &entry) {
  ModelMetaData metadata;
  metadata.name = entry.name;
  metadata.path = entry.getVar("path");
  return metadata;
}

VaoManager *VaoManager::get() {
  static VaoManager *vaomanager = new VaoManager;
  return vaomanager;
}

void VaoManager::importFromFile(const char *path) {
  AssetLoader& loader = AssetLoader::getInstance();

  // Load .impfile from packed assets
  std::string impfileContent = loader.getAssetString(path);
  if (impfileContent.empty()) {
    ERROR("Failed to load model config: %s", path);
    return;
  }

  // Parse in memory
  std::vector<impfile::Entry> entries = impfile::parseBin(impfileContent);

  for (const auto &entry : entries) {
    ModelMetaData metadata = entryToModelMetaData(entry);

    // Load model data from packed assets
    auto modelData = loader.getAssetData(metadata.path.c_str());
    if (modelData.empty()) {
      ERROR("Failed to load model: %s", metadata.path.c_str());
      continue;
    }

    // Write to temp file (fast_obj needs file path)
    // Use cross-platform temp directory (works on Windows, macOS, Linux, Android)
    std::filesystem::path tempDir = std::filesystem::temp_directory_path();
    std::string tempPath = (tempDir / (metadata.name + ".obj")).string();
    std::ofstream tempFile(tempPath, std::ios::binary);
    tempFile.write(reinterpret_cast<const char*>(modelData.data()), modelData.size());
    tempFile.close();

    // Load using existing function
    mesh::Model model = mesh::loadObjModel(tempPath.c_str());
    gfx::Vao vao = gfx::createModelVao(model);
    add(metadata.name, vao);

    // Cleanup
    std::remove(tempPath.c_str());
  }
}

void VaoManager::add(const std::string &name, gfx::Vao vao) {
  vaos.insert({name, vao});
}

void VaoManager::genSimple() {
  add("quad", gfx::createQuadVao());
  add("cube", gfx::createCubeVao());
}

void VaoManager::bind(const std::string &name) {
  if (!vaos.count(name)) {
    ERROR("vao %s does not exist!", name.c_str());
    return;
  }
  vertcount = vaos.at(name).vertcount;
  vaos.at(name).bind();
}

void VaoManager::draw() {
  glDrawElements(GL_TRIANGLES, vertcount, GL_UNSIGNED_INT, 0);
}

void VaoManager::drawInstanced(unsigned int count) {
  glDrawElementsInstanced(GL_TRIANGLES, vertcount, GL_UNSIGNED_INT, 0, count);
}

gfx::Vao &VaoManager::getVao(const std::string &name) {
  if (!vaos.count(name)) {
    ERROR("vao %s does not exist!", name.c_str());
    exit(1);
  }
  return vaos.at(name);
}

FontMetaData entryToFontMetaData(const impfile::Entry &entry) {
  FontMetaData metadata;
  metadata.name = entry.name;
  metadata.path = entry.getVar("path");
  metadata.fontsize = atoi(entry.getVar("fontsz").c_str());
  return metadata;
}

void FontManager::importFromFile(const char *path) {
  AssetLoader& loader = AssetLoader::getInstance();

  std::string impfileContent = loader.getAssetString(path);
  if (impfileContent.empty()) {
    ERROR("Failed to load font config: %s", path);
    return;
  }
  // Parse in memory
  std::vector<impfile::Entry> entries = impfile::parseBin(impfileContent);

  ImGuiIO& io = ImGui::GetIO();
  for (size_t i = 0; i < entries.size(); i++) {
    FontMetaData meta = entryToFontMetaData(entries.at(i));
    INFO("Font name : %s, path : %s", meta.name.c_str(), meta.path.c_str());

    std::vector<uint8_t> fontData = loader.getAssetData(meta.name.c_str());
    if (fontData.empty()) {
      ERROR("Failed to load font data: %s", meta.name.c_str());
      continue;
    }

    // ImGui takes ownership of this buffer, so allocate with IM_ALLOC
    // void* buf = IM_ALLOC(fontData.size());
    // memcpy(buf, fontData.data(), fontData.size());

    // ImFont* font = io.Fonts->AddFontFromMemoryTTF(buf, (int)fontData.size(), (float)meta.fontsize);
    // if (font) {
    //   add(meta.name, font);
    // } else {
    //   ERROR("Failed to register font with ImGui: %s", meta.name.c_str());
    //   IM_FREE(buf);
    // }
  }
}

FontManager *FontManager::get() {
  static FontManager *fontmanager = new FontManager;
  return fontmanager;
}

void FontManager::add(const std::string &name, ImFont* font) {
  fonts.insert({name, font});
}

ImFont* FontManager::getFontData(const std::string &name) {
  if (!fonts.count(name)) {
    ERROR("font %s does not exist!", name.c_str());
    return nullptr;
  }
  return fonts.at(name);
}

void FontManager::pushFont(const std::string &fontname) {
  (void)fontname;

}

void FontManager::popFont() {
  // nk_style_pop_font(State::get()->getNkContext());
}
} // namespace assets
