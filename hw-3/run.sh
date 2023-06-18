#!/bin/bash

# Compile the C program
 make fileSystemOperation
 make makeFileSystem

# echo file
echo "sourceFile" > sourceFile

 #crate file system
  ./makeFileSystem 2 fileSystem.data

# Test 1: dir command
./fileSystemOperation fileSystem.data dir "/"

# Test 2: mkdir command
./fileSystemOperation fileSystem.data mkdir "/ysa/fname"

# Test 3: rmdir command
./fileSystemOperation fileSystem.data rmdir "/ysa/fname"

# Test 4: dumpe2fs command
./fileSystemOperation fileSystem.data dump2fs

# Test 5: write command
./fileSystemOperation fileSystem.data write "/ysa/file" "sourceFile"

# Test 6: read command
./fileSystemOperation fileSystem.data read "/ysa/file" "targetFile"

# Test 7: del command
./fileSystemOperation fileSystem.data del "/ysa/file"

# Clean up - remove the compiled program
rm fileSystemOperation
