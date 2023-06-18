#!/bin/bash

# Inform the user that the compilation is starting
echo "Starting the compilation process..."

# Compile the C program
echo "Compiling fileSystemOperation..."
make fileSystemOperation
echo "Compiling makeFileSystem..."
make makeFileSystem

# Create a source file with some text
echo "Creating a source file with sample text..."
echo "ASDDFASDF" > sourceFile

# Create a file system
echo "Creating a file system..."
./makeFileSystem 2 fileSystem.data

# Test 1: dir command - list directory contents
echo "Test 1: List directory contents (dir command)..."
./fileSystemOperation fileSystem.data dir "/"

# Test 2: mkdir command - create directory
echo "Test 2: Create directory (mkdir command)..."
./fileSystemOperation fileSystem.data mkdir "/ysa/fname"
./fileSystemOperation fileSystem.data dir "/ysa/"

# Test 3: rmdir command - remove directory
echo "Test 3: Remove directory (rmdir command)..."
./fileSystemOperation fileSystem.data rmdir "/ysa/fname"
./fileSystemOperation fileSystem.data dir "/ysa/"

# Test 5: write command - write data to a file
echo "Test 5: Write data to a file (write command)..."
./fileSystemOperation fileSystem.data write "/ysa/file" "sourceFile"

# Test 6: read command - read data from a file
echo "Test 6: Read data from a file (read command)..."
./fileSystemOperation fileSystem.data read "/ysa/file" "targetFile"

# Print the contents of the target file
echo "Contents of the target file:"
cat targetFile
echo "" # Print an empty line for readability

# Test 7: del command - delete a file
echo "Test 7: Delete a file (del command)..."
./fileSystemOperation fileSystem.data del "/ysa/file"

# Test 4: dumpe2fs command - dump filesystem info
echo "Test 4: Dump filesystem info (dumpe2fs command)..."
./fileSystemOperation fileSystem.data dump2fs

# Clean up - remove the compiled program
echo "Cleaning up - Removing the compiled program..."
rm fileSystemOperation

echo "All tasks completed successfully!"