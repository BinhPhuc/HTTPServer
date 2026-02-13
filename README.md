# HTTP Server

A modern C++17 HTTP server implementation using **blocking I/O with thread pool** architecture for handling multiple concurrent connections.

## Overview

This server employs a blocking I/O model combined with a thread pool to handle concurrent requests efficiently. Each connection is processed by a dedicated thread from the pool, enabling the server to serve multiple clients simultaneously.

### Architecture

**Current:** Blocking I/O + Thread Pool  
**Roadmap:** Multiplexer I/O (epoll/kqueue) - Coming Soon

## Prerequisites

- **CMake** 3.20 or higher
- **C++17** compatible compiler (g++, clang++)
- **Git**

## Quick Start

### 1. Clone the Repository

```bash
git clone git@github.com:BinhPhuc/HTTPServer.git
cd HTTPServer
```

### 2. Bootstrap vcpkg

```bash
cd vcpkg
./bootstrap-vcpkg.sh    # Linux/macOS
# or
./bootstrap-vcpkg.bat   # Windows
cd ..
```

### 3. Build and Run

**Recommended:**

```bash
./script/build_and_run.sh
```

This script will automatically:
- Configure CMake with vcpkg integration
- Install dependencies (nlohmann-json)
- Build the project
- Launch the server

**Manual Build:**

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build .
./server
```

## Adding New Dependencies

### Step 1: Search for Package

```bash
cd vcpkg
./vcpkg search <library-name>
# Example: ./vcpkg search boost
```

### Step 2: Update vcpkg.json

Add the package to the `dependencies` array in [vcpkg.json](vcpkg.json):

```json
{
  "name": "httpserver",
  "version": "1.0.0",
  "dependencies": [
    "nlohmann-json",
    "boost-asio",      // Add new package
    "spdlog"           // Multiple packages supported
  ]
}
```

### Step 3: Update CMakeLists.txt

Add the corresponding `find_package` and link commands in [CMakeLists.txt](CMakeLists.txt):

```cmake
# Find packages
find_package(Boost REQUIRED COMPONENTS system)
find_package(spdlog CONFIG REQUIRED)

# Link libraries
target_link_libraries(server PRIVATE 
    nlohmann_json::nlohmann_json
    Boost::system              # New library
    spdlog::spdlog            # New library
)
```

### Step 4: Rebuild

```bash
rm -rf build
./script/build_and_run.sh
```

vcpkg will automatically download and install new dependencies on first build.

**Note:** Package names in vcpkg.json may differ from their `find_package` counterparts. Consult the library's documentation for proper linking instructions.

## Development

### LSP/Clangd Configuration

The build script automatically generates `compile_commands.json` for LSP support (clangd, ccls). This enables:
- Accurate code completion
- Go-to-definition
- Error detection in editors (Neovim, VSCode, etc.)

The `compile_commands.json` file is automatically symlinked to the project root during build.

**Manual setup (if needed):**
```bash
# Generate compile_commands.json
cd build
cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
         -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake

# Create symlink at project root
cd ..
ln -sf build/compile_commands.json .
```