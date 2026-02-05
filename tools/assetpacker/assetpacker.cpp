#include "assetpacker.h"
#include <iostream>
#include <filesystem>

void AssetPacker::scanDirectory(const char* path) {
    std::filesystem::path targetDir = path;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(targetDir)) {

        if (entry.is_directory()) continue; // Skip directories
        if (entry.path().filename() == ".DS_Store") continue; // Skip .DS_Store files
        if (entry.path().extension() == ".impfile") continue; // Skip .impfile files
        if (entry.path().extension() == ".txt") continue; // Skip .txt files

        if (entry.path().extension() == ".png" ||
            entry.path().extension() == ".jpg" ||
            entry.path().extension() == ".jpeg" ||
            entry.path().extension() == ".bmp") {
            textures[entry.path().filename().string()] = entry.path().string();
        } else if (entry.path().extension() == ".vert" ||
                   entry.path().extension() == ".frag" ||
                   entry.path().extension() == ".glsl") {
            shaders[entry.path().filename().string()] = entry.path().string();
        } else if (entry.path().extension() == ".obj" ||
                   entry.path().extension() == ".fbx" ||
                   entry.path().extension() == ".gltf") {
            models[entry.path().filename().string()] = entry.path().string();
        } else if (entry.path().extension() == ".ttf" ||
                   entry.path().extension() == ".otf") {
            fonts[entry.path().filename().string()] = entry.path().string();
        } else if (entry.path().extension() == ".wav" ||
                   entry.path().extension() == ".mp3" ||
                   entry.path().extension() == ".ogg") {
            sounds[entry.path().filename().string()] = entry.path().string();
        } else {
            // Read the file into a byte vector
            // std::ifstream file(entry.path(), std::ios::binary);
            // std::vector<unsigned char> buffer((std::istreambuf_iterator<char>(file)),
            //                                    std::istreambuf_iterator<char>());
            // assets[entry.path().filename().string()] = buffer;
        }
    }

    std::cout << "Textures found: " << textures.size() << std::endl;
    std::cout << "Shaders found: " << shaders.size() << std::endl;
    std::cout << "Models found: " << models.size() << std::endl;
    std::cout << "Fonts found: " << fonts.size() << std::endl;
    std::cout << "Sounds found: " << sounds.size() << std::endl;
}

void AssetPacker::packAssets(const char* outputPath) {
    (void)outputPath;
}
