#!/bin/bash

set -e  

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${YELLOW}========================================${NC}"
echo -e "${YELLOW}  HTTP Server - Build${NC}"
echo -e "${YELLOW}========================================${NC}"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

if [ ! -d "build" ]; then
    echo -e "${YELLOW}Creating build directory...${NC}"
    mkdir -p build
fi

cd build

if [ ! -f "Makefile" ] && [ ! -f "build.ninja" ]; then
    echo -e "${YELLOW}Configuring with CMake...${NC}"
    cmake .. \
        -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
fi

echo -e "${YELLOW}Building project...${NC}"
cmake --build . -j$(nproc)

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Build successful!${NC}"
    
    if [ -f "compile_commands.json" ]; then
        echo -e "${YELLOW}Creating symlink for compile_commands.json...${NC}"
        cd "$PROJECT_ROOT"
        ln -sf build/compile_commands.json .
    fi
else
    echo -e "${RED}✗ Build failed!${NC}"
    exit 1
fi
