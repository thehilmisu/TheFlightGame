#ifndef ASSETPACKER_H
#define ASSETPACKER_H

#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>


typedef enum AssetType {
    ASSET_TYPE_SHADER = 0,
    ASSET_TYPE_TEXTURE = 1,
    ASSET_TYPE_MODEL = 2,
    ASSET_TYPE_MATERIAL = 3,
    ASSET_TYPE_SOUND = 4,
    ASSET_TYPE_FONT = 5,
    ASSET_TYPE_CONFIG = 6

}AssetType;
struct AssetBinHeader {
    uint32_t magic;           // 0x41535442 ("ASTB" in ASCII)
    uint16_t version;         // 1
    uint16_t flags;           // 0 (reserved)
    uint32_t num_assets;      // Total count
    uint64_t index_offset;    // Byte offset to index
    uint64_t data_offset;     // Always 24 (right after header)
};

struct AssetIndexEntry {
    char path[64];           // "assets/shaders/foo.glsl" (null-terminated)
    uint64_t offset;         // Byte offset from file start
    uint64_t size;           // Asset size in bytes
    AssetType type;           // 0=shader, 1=texture, 2=model, 3=sound, 4=font, 5=config
    uint8_t compression;     // 0 (no compression)
    uint8_t reserved[6];     // Padding to 80 bytes
};

class AssetPacker {
    
    public:
        static AssetPacker &getInstance() {
            static AssetPacker instance;
            return instance;
        }

        AssetPacker(const AssetPacker &) = delete;
        AssetPacker &operator=(const AssetPacker &) = delete;

        void scanDirectory(const char* path);
        void packAssets(const char* outputPath);

    private:
        AssetPacker() {}
        ~AssetPacker() {}

        std::unordered_map<std::string, std::string> textures;
        std::unordered_map<std::string, std::string> shaders;
        std::unordered_map<std::string, std::string> models;
        std::unordered_map<std::string, std::string> materials;
        std::unordered_map<std::string, std::string> fonts;
        std::unordered_map<std::string, std::string> sounds;
        std::unordered_map<std::string, std::vector<unsigned char>> assets;

};


#endif //ASSETPACKER_H