#include "assetloader.h"

bool AssetLoader::loadAssets(const std::string& assetBundlePath) {
    m_file.open(assetBundlePath, std::ios::binary);
    if (!m_file) {
        std::cerr << "Failed to open asset bundle: " << assetBundlePath << std::endl;
        return false;
    }
    std::cout << "Loading asset bundle: " << assetBundlePath << std::endl;
    AssetBinHeader header;
    m_file.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (header.magic != 0x41535442) {
        std::cerr << "Invalid asset bundle magic!" << std::endl;
        return false;
    }

    // Jump to the index table
    m_file.seekg(header.index_offset);

    // Read all index entries
    for (uint32_t i = 0; i < header.num_assets; ++i) {
        AssetIndexEntry entry;
        m_file.read(reinterpret_cast<char*>(&entry), sizeof(entry));

        std::string assetPath = entry.path;  // Ensure null-terminated string
        m_index[assetPath] = { entry.offset, entry.size, entry.type };

        // Debug: print first few assets and impfiles
        if (i < 10 || entry.type == ASSET_TYPE_IMPFILE) {
            std::cout << "Loaded asset: [" << assetPath << "] type=" << (int)entry.type << std::endl;
        }
    }

    std::cout << "Total assets loaded: " << m_index.size() << std::endl;
    return true;
}

std::vector<uint8_t> AssetLoader::getAssetData(const std::string& name) {
    if (m_index.find(name) == m_index.end()) {
        std::cerr << "Asset not found: " << name << std::endl;
        return {};
    }

    const auto& info = m_index[name];
    std::vector<uint8_t> buffer(info.size);

    m_file.seekg(info.offset);
    m_file.read(reinterpret_cast<char*>(buffer.data()), info.size);

    return buffer;
}
