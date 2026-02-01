#!/bin/bash
# Build script for ChatGPT Desktop

set -e  # Exit on error

echo "======================================"
echo "ChatGPT Desktop - Build Script"
echo "======================================"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check for required tools
echo -e "${YELLOW}Checking dependencies...${NC}"

if ! command -v cmake &> /dev/null; then
    echo -e "${RED}ERROR: cmake not found. Install with: sudo zypper install cmake${NC}"
    exit 1
fi

if ! command -v qmake &> /dev/null && ! command -v qmake-qt5 &> /dev/null; then
    echo -e "${RED}ERROR: Qt5 not found. Install with: sudo zypper install qt5-qtbase-devel${NC}"
    exit 1
fi

echo -e "${GREEN}✓ All dependencies found${NC}"

# Clean previous build
if [ -d "build" ]; then
    echo -e "${YELLOW}Cleaning previous build...${NC}"
    rm -rf build
fi

# Create build directory
echo -e "${YELLOW}Creating build directory...${NC}"
mkdir -p build
cd build

# Run CMake
echo -e "${YELLOW}Running CMake...${NC}"
cmake .. || {
    echo -e "${RED}ERROR: CMake configuration failed${NC}"
    exit 1
}

# Build
echo -e "${YELLOW}Building application...${NC}"
make -j$(nproc) || {
    echo -e "${RED}ERROR: Build failed${NC}"
    exit 1
}

# Check if binary was created
if [ -f "boh_chat_desktop" ]; then
    echo ""
    echo -e "${GREEN}======================================"
    echo "✓ Build successful!"
    echo "======================================${NC}"
    echo ""
    echo "Executable: $(pwd)/boh_chat_desktop"
    echo "Size: $(du -h boh_chat_desktop | cut -f1)"
    echo ""
    echo "To run the application:"
    echo "  ./build/boh_chat_desktop"
    echo ""
    echo "To install system-wide:"
    echo "  sudo ./install.sh"
    echo ""
else
    echo -e "${RED}ERROR: Binary not found after build${NC}"
    exit 1
fi

