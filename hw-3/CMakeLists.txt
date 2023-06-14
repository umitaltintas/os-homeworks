cmake_minimum_required(VERSION 3.10)
project(FileSystemProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the include directory to the include path
include_directories(${PROJECT_SOURCE_DIR}/include)

# Add executable for fileSystemOperation
add_executable(fileSystemOperation
        ${PROJECT_SOURCE_DIR}/src/fileSystemOperation.cpp
        ${PROJECT_SOURCE_DIR}/src/FileSystem.cpp
        )

# Add executable for makeFileSystem
add_executable(makeFileSystem
        ${PROJECT_SOURCE_DIR}/src/makeFileSystem.cpp
        ${PROJECT_SOURCE_DIR}/src/FileSystem.cpp
        )
