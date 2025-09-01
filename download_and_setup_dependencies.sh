#!/bin/bash

# Shape-Force - Dependency Download Script
# This script downloads and extracts all required third-party libraries

set -e  # Exit on any error

# Colors for output
# Use: https://www.ubuntumint.com/linux-echo-color/ for reference
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Detect platform
detect_platform() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        PLATFORM="linux"
    else
        print_error "Unsupported platform: $OSTYPE"
        exit 1
    fi
    print_status "Detected platform: $PLATFORM"
}

# Create directory structure
create_directories() {
    print_status "Creating directory structure..."
    
    mkdir -p thirdparties/$PLATFORM/{imgui,glfw,gl3w,spd_log}
    mkdir -p build
    mkdir -p src/imgui
    
    if [[ "$PLATFORM" == "windows" ]]; then
        mkdir -p thirdparties/$PLATFORM/glew
    fi
    
    print_success "Directory structure created"
}

# Download and extract Dear ImGui
setup_imgui() {
    print_status "Setting up Dear ImGui..."
    
    IMGUI_VERSION="v1.90.1"
    IMGUI_URL="https://github.com/ocornut/imgui/archive/refs/tags/$IMGUI_VERSION.tar.gz"
    cd thirdparties/$PLATFORM/imgui
    
    if [[ ! -f "imgui.cpp" ]]; then
        print_status "Downloading Dear ImGui $IMGUI_VERSION..."
        curl -L "$IMGUI_URL" -o imgui.tar.gz
        tar -xzf imgui.tar.gz --strip-components=1
        rm imgui.tar.gz
        print_success "Dear ImGui downloaded and extracted"
    else
        print_warning "Dear ImGui already exists, skipping download"
    fi
    
    cd - > /dev/null
}

# Download and extract GLFW
setup_glfw() {
    print_status "Setting up GLFW..."

    GLFW_VERSION="3.4"
    GLFW_VERSION="https://github.com/glfw/glfw/archive/refs/tags/$GLFW_VERSION.tar.gz"
    cd thirdparties/$PLATFORM/glfw
    
    if [[ "$PLATFORM" == "linux" ]]; then
        print_status "Downloading GLFW $GLFW_VERSION..."
        curl -L "$GLFW_VERSION" -o glfw.tar.gz
        tar -xzf glfw.tar.gz --strip-components=1
        rm glfw.tar.gz
        print_success "GLFW downloaded and extracted"

        # Check if Python is available
        if command -v cmake &> /dev/null; then
            print_status "Building GLFW $GLFW_VERSION..."
            mkdir -p build
            cmake -S . -B ./build
            cmake --build ./build
        else
            print_error "Cmake is required to build glfw, please ensure to run this script inside Docker container 'map_dev_env:latest'"
            exit 1
        fi


    else
        print_error "Unsupported platform: $OSTYPE"
        exit 1
    fi

    cd - > /dev/null
}

# Setup GL3W (Linux/Mac only)
setup_gl3w() {
    if [[ "$PLATFORM" == "windows" ]]; then
        return  # Skip GL3W on Windows (use GLEW instead)
    fi
    
    print_status "Setting up GL3W..."
    
    cd thirdparties/$PLATFORM/gl3w
    
    if [[ ! -f "src/gl3w.c" ]]; then
        print_status "Downloading GL3W..."
        git clone https://github.com/skaslev/gl3w.git temp_gl3w
        cd temp_gl3w
        
        # Check if Python is available
        if command -v python3 &> /dev/null; then
            python3 gl3w_gen.py
        elif command -v python &> /dev/null; then
            python gl3w_gen.py
        else
            print_error "Python is required to generate GL3W files"
            exit 1
        fi
        
        cd ..
        cp -r temp_gl3w/include .
        cp -r temp_gl3w/src .
        
        print_success "GL3W generated and extracted"
    else
        print_warning "GL3W already exists, skipping generation"
    fi

    # Go back to thirdpaties level
    cd ../../..
}

setup_glaze() {
    print_status "Start glaze json download and setup"
}

setup_spdlog(){
    print_status "Start spdlog download and setup..."
    SPD_LOG_VERSION="1.15.3"
    SPD_LOG_VERSION="https://github.com/gabime/spdlog/archive/refs/tags/v$SPD_LOG_VERSION.tar.gz"
    cd thirdparties/$PLATFORM/spd_log

    if [[ "$PLATFORM" == "linux" ]]; then
        print_status "Downloading SPD_LOG $SPD_LOG_VERSION..."
        curl -L "$SPD_LOG_VERSION" -o spd_log.tar.gz
        tar -xzf spd_log.tar.gz --strip-components=1
        rm spd_log.tar.gz
        print_success "SPD_LOG downloaded and extracted"

        # Check if Python is available
        if command -v cmake &> /dev/null; then
            print_status "Building SPD_LOG $SPD_LOG_VERSION..."
            mkdir -p build
            cmake -S . -B ./build
            cmake --build ./build
        else
            print_error "Cmake is required to build sdp log, please ensure to run this script inside Docker container 'map_dev_env:latest'"
            exit 1
        fi


    else
        print_error "Unsupported platform: $OSTYPE"
        exit 1
    fi
    cd - > /dev/null
}

# Main execution
main() {
    echo "Shape-Force - Dependency Setup"
    echo "===================================================="
    echo
    
    detect_platform
    create_directories
    
    print_status "Starting dependency download and setup..."
    
    setup_imgui
    setup_glfw
    setup_gl3w

    # Setup glazed and spdlog
    setup_glaze
    setup_spdlog
    echo
    
    echo
    print_success "Dependency setup completed!"
    print_status "You can now build the project with:"
    echo "  mkdir -p build && cd build"
    echo "  cmake .."
    echo "  cmake --build ."
    echo
}

# Run main function
main "$@"