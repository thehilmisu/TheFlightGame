#include <iostream>
#include <ostream>
#include <filesystem>
#include "assetpacker.h"


int main(int argc, char** argv) {

    // Usage: AssetPacker [assets_dir] [output_file]
    // These used to be ignored, so the packer always wrote ./assets.bin
    // relative to its working directory instead of the path the build system
    // asked for. The build then believed the bundle lived in the build
    // directory, found a stale copy there, and skipped repacking entirely.
    const char* path = argc > 1 ? argv[1] : "assets";
    const char* output = argc > 2 ? argv[2] : "assets.bin";

    std::cout << "########################### " << std::endl;
    std::cout << "# Asset Packer STARTED !" << std::endl;
    std::cout << "########################### " << std::endl;
    std::cout << "Output: " << output << std::endl;

    AssetPacker &assetPacker = AssetPacker::getInstance();
    assetPacker.scanDirectory(path);
    assetPacker.packAssets(output);

    std::cout << "########################### " << std::endl;
    std::cout << "# Asset Packer DONE !" << std::endl;
    std::cout << "########################### " << std::endl;
}