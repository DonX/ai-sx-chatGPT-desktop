#!/bin/bash
# RPM Build Script for ChatGPT Desktop
# For openSUSE Leap / Tumbleweed

set -e  # Exit on error

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}======================================"
echo "ChatGPT Desktop - RPM Build Script"
echo "======================================${NC}"
echo ""

# Configuration
NAME="chatgpt-desktop"
VERSION="0.1.0"
RELEASE="1"
SPEC_FILE="chatgpt-desktop.spec"

# Check if running on openSUSE
if [ ! -f /etc/os-release ]; then
    echo -e "${RED}ERROR: Cannot detect OS${NC}"
    exit 1
fi

source /etc/os-release
if [[ ! "$ID" =~ ^(opensuse|suse)$ ]]; then
    echo -e "${YELLOW}WARNING: This script is designed for openSUSE${NC}"
    echo "Current OS: $PRETTY_NAME"
    read -p "Continue anyway? (y/N) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

echo -e "${GREEN}✓ Detected: $PRETTY_NAME${NC}"
echo ""

# Check for required tools
echo -e "${YELLOW}Checking build dependencies...${NC}"

MISSING_DEPS=()

if ! command -v rpmbuild &> /dev/null; then
    MISSING_DEPS+=("rpm-build")
fi

if ! command -v cmake &> /dev/null; then
    MISSING_DEPS+=("cmake")
fi

if ! command -v qmake-qt5 &> /dev/null && ! command -v qmake &> /dev/null; then
    MISSING_DEPS+=("qt5-qtbase-devel")
fi

if [ ${#MISSING_DEPS[@]} -ne 0 ]; then
    echo -e "${RED}ERROR: Missing dependencies:${NC}"
    for dep in "${MISSING_DEPS[@]}"; do
        echo "  - $dep"
    done
    echo ""
    echo "Install with:"
    echo -e "${YELLOW}  sudo zypper install ${MISSING_DEPS[*]}${NC}"
    exit 1
fi

echo -e "${GREEN}✓ All build dependencies found${NC}"
echo ""

# Run security check
if [ -f "check-security.sh" ]; then
    echo -e "${YELLOW}Running security check...${NC}"
    if ! ./check-security.sh; then
        echo -e "${RED}ERROR: Security check failed!${NC}"
        echo "Fix the issues before building RPM"
        exit 1
    fi
    echo ""
fi

# Setup RPM build environment
echo -e "${YELLOW}Setting up RPM build environment...${NC}"
rpmdev-setuptree 2>/dev/null || mkdir -p ~/rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
echo -e "${GREEN}✓ RPM build tree ready${NC}"
echo ""

# Create source tarball
echo -e "${YELLOW}Creating source tarball...${NC}"
TARBALL="${NAME}-${VERSION}.tar.gz"
TEMP_DIR=$(mktemp -d)
SRC_DIR="${TEMP_DIR}/${NAME}-${VERSION}"

mkdir -p "$SRC_DIR"

# Copy source files (exclude build artifacts and user data)
rsync -a \
    --exclude='build/' \
    --exclude='build-*/' \
    --exclude='.git/' \
    --exclude='*.o' \
    --exclude='*.db' \
    --exclude='*.conf' \
    --exclude='*.user' \
    --exclude='*.autosave' \
    --exclude='*.rpm' \
    --exclude='*.tar.gz' \
    --exclude='CMakeLists.txt.user' \
    --exclude='*_autogen/' \
    ./ "$SRC_DIR/"

# Create tarball
cd "$TEMP_DIR"
tar czf "$TARBALL" "${NAME}-${VERSION}/"
mv "$TARBALL" ~/rpmbuild/SOURCES/
cd - > /dev/null

# Cleanup temp directory
rm -rf "$TEMP_DIR"

echo -e "${GREEN}✓ Source tarball created: ~/rpmbuild/SOURCES/$TARBALL${NC}"
echo ""

# Copy spec file
echo -e "${YELLOW}Copying spec file...${NC}"
cp "$SPEC_FILE" ~/rpmbuild/SPECS/
echo -e "${GREEN}✓ Spec file copied${NC}"
echo ""

# Build RPM
echo -e "${YELLOW}Building RPM package...${NC}"
echo -e "${BLUE}This may take a few minutes...${NC}"
echo ""

cd ~/rpmbuild/SPECS
if rpmbuild -ba "$SPEC_FILE"; then
    echo ""
    echo -e "${GREEN}======================================"
    echo "✓ RPM BUILD SUCCESSFUL!"
    echo "======================================${NC}"
    echo ""
    echo "Binary RPM:"
    ls -lh ~/rpmbuild/RPMS/x86_64/${NAME}-${VERSION}-${RELEASE}.*.rpm 2>/dev/null || \
        ls -lh ~/rpmbuild/RPMS/x86_64/${NAME}-${VERSION}*.rpm
    echo ""
    echo "Source RPM:"
    ls -lh ~/rpmbuild/SRPMS/${NAME}-${VERSION}-${RELEASE}.*.src.rpm 2>/dev/null || \
        ls -lh ~/rpmbuild/SRPMS/${NAME}-${VERSION}*.src.rpm
    echo ""
    echo -e "${YELLOW}To install:${NC}"
    echo "  sudo zypper install ~/rpmbuild/RPMS/x86_64/${NAME}-${VERSION}*.rpm"
    echo ""
    echo -e "${YELLOW}To test before installing:${NC}"
    echo "  rpm -qpl ~/rpmbuild/RPMS/x86_64/${NAME}-${VERSION}*.rpm"
    echo ""
else
    echo ""
    echo -e "${RED}======================================"
    echo "✗ RPM BUILD FAILED"
    echo "======================================${NC}"
    echo ""
    echo "Check the error messages above for details"
    exit 1
fi

