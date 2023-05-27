# Set the compiler
UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
	CXX = clang++
else
	CXX = g++
endif

# Set the C++ standard
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic

# Set the build directory
BUILD_DIR = ./build

# Set the source directory
SRC_DIR = ./src

# Set the include directory
INC_DIR = ./include

# Specify the target
TARGET = memory_management

# Specify the source files
SRC = $(wildcard $(SRC_DIR)/*.cpp)

# Specify the object files
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

ifdef SHOW_MEMORY
	CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer
endif

$(BUILD_DIR)/$(TARGET): $(OBJ)
	$(CXX) $^ -o $@ $(CXXFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(BUILD_DIR)
	$(CXX) -c $< -o $@ $(CXXFLAGS) -I$(INC_DIR)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

run: $(BUILD_DIR)/$(TARGET)
	./$(BUILD_DIR)/$(TARGET) $(ARGS)