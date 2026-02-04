 cmake_minimum_required(VERSION 3.10)
 project(AssetPacker)

 set(CMAKE_CXX_STANDARD 17)
 set(CMAKE_CXX_STANDARD_REQUIRED ON)

 add_executable(AssetPacker
         main.cpp
         assetpacker.cpp
         assetpacker.h
 )

 # Add compiler warnings
 if(MSVC)
     target_compile_options(AssetPacker PRIVATE /W4)
 else()
     target_compile_options(AssetPacker PRIVATE -Wall -Wextra)
 endif()