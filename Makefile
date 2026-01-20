# Target executable name
TARGET = RiverRaid3D.exe

# Project structure
SRC = main.cpp src/glad.c src/window.cpp src/gui.cpp src/shader.cpp \
	  src/noise.cpp src/stb_image_impl.c src/fast_obj.c src/gfx.cpp \
	  src/geometry.cpp src/infworld.cpp src/chunktable.cpp src/chunkdecorations.cpp \
	  src/assets.cpp src/importfile.cpp src/camera.cpp src/game.cpp src/plants.cpp src/arcade_mode.cpp src/menu.cpp \
	  src/display.cpp src/player.cpp src/update.cpp \
	  thirdparty/imgui/imgui.cpp \
	  thirdparty/imgui/imgui_draw.cpp \
	  thirdparty/imgui/imgui_widgets.cpp \
	  thirdparty/imgui/imgui_tables.cpp \
	  thirdparty/imgui/backends/imgui_impl_glfw.cpp \
	  thirdparty/imgui/backends/imgui_impl_opengl3.cpp \
	  
INC_DIR =  -Iinclude
INC_DIR += -Iinclude/glad
INC_DIR += -Iinclude/KHR
INC_DIR += -Ithirdparty/imgui
INC_DIR += -Ithirdparty/imgui/backends
INC_DIR += -Ithirdparty/glm
INC_DIR += -Isrc

BUILD_DIR = build

# OS Detection Logic
ifeq ($(OS),Windows_NT)
    # --- Windows Settings (MinGW) ---
    LDFLAGS = -Llib-mingw-w64 -pthread -lglfw3 -lopengl32 -lgdi32 -luser32 -lshell32
    CLEAN_CMD = rm $(BUILD_DIR)/$(TARGET)
    FIX_PATH = $(subst /,\,$1)
else
	TARGET = RiverRaid3D
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        # --- macOS Settings ---
        LIBDIRS = -L/opt/homebrew/lib \
		          -L/usr/local/lib
		          
        LDFLAGS = $(LIBDIRS) -lglfw -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
        CLEAN_CMD = rm -f $(BUILD_DIR)/$(TARGET)
        FIX_PATH = $1
    else
        # --- Linux Settings ---
        LDFLAGS = -Llib -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl
        CLEAN_CMD = rm -f $(BUILD_DIR)/$(TARGET)
        FIX_PATH = $1
    endif
endif

# Compiler settings
CXX = g++
CXXFLAGS = $(INC_DIR) -Wall -std=c++17

# Build Rules
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) $(LDFLAGS) -o $(BUILD_DIR)/$(TARGET)

build_dir:
	mkdir -p $(BUILD_DIR)

clean:
	$(CLEAN_CMD)

.phony:
	build_dir
