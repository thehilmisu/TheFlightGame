#include "assetpacker.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <filesystem>

void AssetPacker::scanDirectory(const char* path) {
    std::cout << "Scanning assets directory: " << path << std::endl;
    std::filesystem::path targetDir = std::filesystem::absolute(path);
    std::filesystem::path parentDir = targetDir.parent_path();

    for (const auto& entry : std::filesystem::recursive_directory_iterator(targetDir)) {

        if (entry.is_directory()) continue; // Skip directories
        if (entry.path().filename() == ".DS_Store") continue; // Skip .DS_Store files
        if (entry.path().extension() == ".txt") continue; // Skip .txt files

        // Use just the filename (no directory path)
        std::string filename = entry.path().filename().string();

        if (entry.path().extension() == ".png" ||
            entry.path().extension() == ".jpg" ||
            entry.path().extension() == ".jpeg" ||
            entry.path().extension() == ".bmp") {
            textures[filename] = entry.path().string();
        } else if (entry.path().extension() == ".vert" ||
                   entry.path().extension() == ".frag" ||
                   entry.path().extension() == ".glsl") {
            shaders[filename] = entry.path().string();
        } else if (entry.path().extension() == ".obj" ||
                   entry.path().extension() == ".fbx" ||
                   entry.path().extension() == ".gltf") {
            models[filename] = entry.path().string();
        } else if (entry.path().extension() == ".mtl") {
            materials[filename] = entry.path().string();
        } else if (entry.path().extension() == ".ttf" ||
                   entry.path().extension() == ".otf") {
            fonts[filename] = entry.path().string();
        } else if (entry.path().extension() == ".wav" ||
                   entry.path().extension() == ".mp3" ||
                   entry.path().extension() == ".ogg") {
            sounds[filename] = entry.path().string();
        } else if (entry.path().extension() == ".impfile") {
            impfiles[filename] = entry.path().string();
        } else {
            std::cout << "Unknown asset type: " << entry.path() << std::endl;
        }
    }

    std::cout << "Textures found: " << textures.size() << std::endl;
    std::cout << "Shaders found: " << shaders.size() << std::endl;
    std::cout << "Models found: " << models.size() << std::endl;
    std::cout << "Materials found: " << materials.size() << std::endl;
    std::cout << "Fonts found: " << fonts.size() << std::endl;
    std::cout << "Sounds found: " << sounds.size() << std::endl;
    std::cout << "Impfiles found: " << impfiles.size() << std::endl;
}

void AssetPacker::packAssets(const char* outputPath) {
    std::cout << "Start packing assets  " << std::endl;
    // Temporary list of all assets with their assigned types
    struct InternalEntry {
        std::string name;
        std::string fullPath;
        AssetType type;
    };
    std::vector<InternalEntry> allAssets;

    auto add = [&](auto& map, AssetType type) {
        for (const auto& [name, path] : map) allAssets.push_back({name, path, type});
    };

    add(shaders, ASSET_TYPE_SHADER);
    add(textures, ASSET_TYPE_TEXTURE);
    add(models, ASSET_TYPE_MODEL);
    add(materials, ASSET_TYPE_MATERIAL);
    add(sounds, ASSET_TYPE_SOUND);
    add(fonts, ASSET_TYPE_FONT);
    add(impfiles, ASSET_TYPE_IMPFILE); 

    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile) return;

    // Setup the Main Header
    AssetBinHeader binHeader;
    binHeader.magic = 0x41535442;
    binHeader.version = 1;
    binHeader.flags = 0;
    binHeader.num_assets = static_cast<uint32_t>(allAssets.size());
    binHeader.data_offset = sizeof(AssetBinHeader); // Data starts right after header (24 bytes)
    
    // We can't know the index_offset until we know the total size of all asset data
    // So we leave it as 0 for now and seek back later.
    outFile.write(reinterpret_cast<const char*>(&binHeader), sizeof(binHeader));

    // Write Asset Data
    std::vector<AssetIndexEntry> indexEntries;
    uint64_t currentOffset = binHeader.data_offset;

    for (const auto& asset : allAssets) {
        std::ifstream inFile(asset.fullPath, std::ios::binary);
        if (!inFile) continue;

        uint64_t size = std::filesystem::file_size(asset.fullPath);

        // Create the index entry for this asset
        AssetIndexEntry entry{};
        std::strncpy(entry.path, asset.name.c_str(), sizeof(entry.path) - 1);
        entry.offset = currentOffset;
        entry.size = size;
        entry.type = asset.type;
        entry.compression = 0; // No compression

        // Write the actual file data to the .bin
        outFile << inFile.rdbuf();

        indexEntries.push_back(entry);
        currentOffset += size;
    }

    // Write the Index Table
    uint64_t indexLocation = outFile.tellp(); // Current position is the start of the index
    outFile.write(reinterpret_cast<const char*>(indexEntries.data()), indexEntries.size() * sizeof(AssetIndexEntry));

    // Finalize the Header
    // Go back to the beginning and update the index_offset
    binHeader.index_offset = indexLocation;
    outFile.seekp(0); 
    outFile.write(reinterpret_cast<const char*>(&binHeader), sizeof(binHeader));

    outFile.close();
    std::cout << "Packed " << binHeader.num_assets << " assets into " << outputPath << std::endl;

    auto totalSize = std::filesystem::file_size(outputPath);
    if (totalSize >= 1024 * 1024) {
        std::cout << "Total Size: " << std::fixed << std::setprecision(2) 
                  << (static_cast<double>(totalSize) / (1024.0 * 1024.0)) << " MB" << std::endl;
    } else {
        std::cout << "Total Size: " << (totalSize / 1024) << " KB" << std::endl;
    }
}
