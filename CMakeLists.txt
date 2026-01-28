cmake_minimum_required(VERSION 3.10)
project(RiverRaid3D)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform detection
if(ANDROID)
    message(STATUS "Building for Android")
    set(PLATFORM_NAME "Android")
else()
    message(STATUS "Building for Desktop")
    set(PLATFORM_NAME "Desktop")
endif()

# Source files (common to all platforms)
set(COMMON_SOURCES
    main.cpp
    src/window.cpp
    src/gui.cpp
    src/shader.cpp
    src/noise.cpp
    src/stb_image_impl.c
    src/fast_obj.c
    src/gfx.cpp
    src/geometry.cpp
    src/infworld.cpp
    src/chunktable.cpp
    src/chunkdecorations.cpp
    src/assets.cpp
    src/importfile.cpp
    src/camera.cpp
    src/game.cpp
    src/plants.cpp
    src/arcade_mode.cpp
    src/menu.cpp
    src/balloon.cpp
    src/ship.cpp
    src/enemies.cpp
    src/props.cpp
    src/barrel.cpp
    src/dev_mode.cpp
    src/display.cpp
    src/player.cpp
    src/update.cpp
)

# ImGui sources
set(IMGUI_SOURCES
    thirdparty/imgui/imgui.cpp
    thirdparty/imgui/imgui_draw.cpp
    thirdparty/imgui/imgui_widgets.cpp
    thirdparty/imgui/imgui_tables.cpp
    thirdparty/imgui/backends/imgui_impl_opengl3.cpp
)

# Include directories (common to all platforms)
set(COMMON_INCLUDES
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/KHR
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/backends
    ${CMAKE_SOURCE_DIR}/src
)

# Platform-specific configuration
if(ANDROID)
    # Android-specific settings
    add_library(${PROJECT_NAME} SHARED ${COMMON_SOURCES} ${IMGUI_SOURCES})

    # Add SDL2 backend for ImGui
    target_sources(${PROJECT_NAME} PRIVATE
        thirdparty/imgui/backends/imgui_impl_sdl2.cpp
    )

    # Find SDL2
    find_library(SDL2_LIBRARY SDL2)

    # Android libraries
    find_library(log-lib log)
    find_library(android-lib android)
    find_library(EGL-lib EGL)
    find_library(GLESv3-lib GLESv3)

    target_link_libraries(${PROJECT_NAME}
        ${SDL2_LIBRARY}
        ${log-lib}
        ${android-lib}
        ${EGL-lib}
        ${GLESv3-lib}
    )

    # Define Android platform
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        PLATFORM_ANDROID
        USE_SDL2
        GL_GLEXT_PROTOTYPES
    )

else()
    # Desktop-specific settings (SDL2 only)
    add_executable(${PROJECT_NAME} ${COMMON_SOURCES} ${IMGUI_SOURCES})

    # SDL2 backend for ImGui
    target_sources(${PROJECT_NAME} PRIVATE
        thirdparty/imgui/backends/imgui_impl_sdl2.cpp
    )

    # Find SDL2 - use pkg-config on Unix systems
    if(APPLE OR UNIX)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(SDL2 REQUIRED sdl2)

        target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
        target_link_directories(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARY_DIRS})
        target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
    else()
        # Windows - try find_package
        find_package(SDL2 REQUIRED)
        target_link_libraries(${PROJECT_NAME} SDL2::SDL2)
    endif()

    message(STATUS "Using SDL2 backend")
    message(STATUS "SDL2 include dirs: ${SDL2_INCLUDE_DIRS}")
    message(STATUS "SDL2 libraries: ${SDL2_LIBRARIES}")

    # Platform-specific libraries
    if(APPLE)
        target_link_libraries(${PROJECT_NAME}
            "-framework OpenGL"
            "-framework Cocoa"
            "-framework IOKit"
            "-framework CoreVideo"
        )
    elseif(WIN32)
        target_link_libraries(${PROJECT_NAME}
            opengl32
            gdi32
            user32
            shell32
        )
    else() # Linux
        find_package(OpenGL REQUIRED)
        target_link_libraries(${PROJECT_NAME}
            OpenGL::GL
            X11
            Xrandr
            Xi
            dl
            pthread
        )
    endif()
endif()

# Common include directories for all platforms
target_include_directories(${PROJECT_NAME} PRIVATE ${COMMON_INCLUDES})

# Compiler warnings
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
endif()

# Copy assets to build directory (desktop only)
if(NOT ANDROID)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
        COMMENT "Copying assets to build directory"
    )
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Platform: ${PLATFORM_NAME}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
