CXX = g++
CXXFLAGS = -ggdb -pedantic-errors -Wall -Weffc++ -Wextra -Wconversion -Wsign-conversion -std=c++17 -Iinclude
TARGET = build/server
SRC_DIR = src
BUILD_DIR = build

SRCS = $(shell find $(SRC_DIR) -name "*.cpp")

all: $(TARGET)

$(TARGET): $(SRCS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -rf $(BUILD_DIR)
