#!/bin/bash

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${YELLOW}========================================${NC}"
echo -e "${YELLOW}  HTTP Server - Run${NC}"
echo -e "${YELLOW}========================================${NC}"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

if [ ! -f "build/server" ]; then
    echo -e "${RED}✗ Server executable not found!${NC}"
    echo -e "${YELLOW}Please run build.sh first.${NC}"
    exit 1
fi

echo -e "${GREEN}Starting server...${NC}"
./build/server
