#ifndef ASSETLOADER_H
#define ASSETLOADER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include "assetpacker.h"
#include <cinttypes>
#include <string>

class AssetLoader {
public:
    static AssetLoader &getInstance() {
        static AssetLoader instance;
        return instance;
    }
    AssetLoader(const AssetLoader &) = delete;
    AssetLoader &operator=(const AssetLoader &) = delete;

    struct AssetInfo {
        uint64_t offset;
        uint64_t size;
        AssetType type;
    };

    bool loadAssets(const std::string& assetBundlePath);

    // Get the raw bytes for an asset
    std::vector<uint8_t> getAssetData(const std::string& name);

    AssetInfo getInfo(const std::string& name) { return m_index[name]; }

private:
    AssetLoader() {}
    ~AssetLoader() {}
    std::ifstream m_file;
    std::unordered_map<std::string, AssetInfo> m_index;
};

#endif