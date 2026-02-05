#ifndef ASSETPACKER_H
#define ASSETPACKER_H

#include <unordered_map>
#include <vector>
#include <string>

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
        std::unordered_map<std::string, std::string> fonts;
        std::unordered_map<std::string, std::string> sounds;
        std::unordered_map<std::string, std::vector<unsigned char>> assets;

};


#endif //ASSETPACKER_H