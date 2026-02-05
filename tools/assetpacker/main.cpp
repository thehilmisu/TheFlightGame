#include <iostream>
#include <ostream>
#include <filesystem>
#include "assetpacker.h"


int main(int argc, char** argv) {
    
    (void)argc;
    (void)argv;

    std::cout << "########################### " << std::endl;
    std::cout << "# Asset Packer STARTED !" << std::endl;
    std::cout << "########################### " << std::endl;

    const char* path = "assets";
    AssetPacker &assetPacker = AssetPacker::getInstance();
    assetPacker.scanDirectory(path);
    assetPacker.packAssets("assets.bin");

    std::cout << "########################### " << std::endl;
    std::cout << "# Asset Packer DONE !" << std::endl;
    std::cout << "########################### " << std::endl;
}