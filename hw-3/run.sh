#!/bin/bash

# Function to test the program with given parameters
# Function to test the program with given parameters
test_program() {
    frame_size=$1
    physical_frames=$2
    virtual_frames=$3
    algorithm=$4
    table_type=$5
    print_t=$6
    disk_file=$7

    echo "Testing with parameters: $frame_size $physical_frames $virtual_frames $algorithm $table_type $print_t $disk_file"

    ./build/memory_management $frame_size $physical_frames $virtual_frames $algorithm $table_type $print_t $disk_file
}

# Test cases
test_program 12 5 10 WSC inverted 10000 diskFileName.dat
test_program 10 3 7 SC flat 10000 anotherDiskFile.dat

# Add more test cases as needed
aw