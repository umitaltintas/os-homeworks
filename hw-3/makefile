# Variables
CXX = g++
CXXFLAGS = -std=c++17 -Wno-ignored-attributes
INCLUDES = -I./include
SOURCES_DIR = ./src
BUILD_DIR = ./build

# Required to create the build directory if it doesn't exist
$(shell mkdir -p $(BUILD_DIR))

# Executable 1: fileSystemOperation
fileSystemOperation_SRC = $(SOURCES_DIR)/fileSystemOperation.cpp $(SOURCES_DIR)/FileSystem.cpp
fileSystemOperation_OBJ = $(patsubst $(SOURCES_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(fileSystemOperation_SRC))

fileSystemOperation: $(fileSystemOperation_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(INCLUDES)

# Executable 2: makeFileSystem
makeFileSystem_SRC = $(SOURCES_DIR)/makeFileSystem.cpp $(SOURCES_DIR)/FileSystem.cpp
makeFileSystem_OBJ = $(patsubst $(SOURCES_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(makeFileSystem_SRC))

makeFileSystem: $(makeFileSystem_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(INCLUDES)

# Generic rule for turning .cpp into .o
$(BUILD_DIR)/%.o: $(SOURCES_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INCLUDES)

# Cleanup
.PHONY: clean
clean:
	rm -f $(BUILD_DIR)/*.o fileSystemOperation makeFileSystem
