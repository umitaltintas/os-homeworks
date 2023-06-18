#ifndef HW_3_FILESYSTEM_H
#define HW_3_FILESYSTEM_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>

// Attribute constants
#define ATTR_DIRECTORY 0x10
#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_ARCHIVE 0x20


#define BLOCK_COUNT 2880
#define ROOT_DIR_ENTRIES_COUNT 14

// 12-bit unsigned integer
typedef struct {
    unsigned int value: 12;
} uint_12 __attribute__((packed));

// FAT12 boot sector structure
typedef struct {
    unsigned short block_size;// block size
    unsigned short block_count; // number of blocks
    unsigned short root_dir_entries_count; // number of entries in the root directory
} boot_sector_t __attribute__((packed));

// FAT12 directory entry structure
typedef struct {
    unsigned char filename[8];
    unsigned char ext[3];
    unsigned char attr;
    unsigned short time;
    unsigned short date;
    uint_12 starting_cluster;
    unsigned int file_size;
} dir_entry_t __attribute__((packed));


// FAT12 filesystem structure
typedef struct {
    FILE *file;
    boot_sector_t boot_sector;
    uint_12 *fat;
    dir_entry_t *root_dir;
} fat12_t __attribute__((packed));

// Function declarations for boot sector
boot_sector_t read_boot_sector(FILE *file);

void write_boot_sector(FILE *file, boot_sector_t boot);

// Function declarations for FAT
uint_12 *read_fat(FILE *file, boot_sector_t boot);

void write_fat(FILE *file, boot_sector_t, uint_12 *fat);

// Function declarations for root directory
dir_entry_t *read_root_dir(FILE *file, boot_sector_t boot);

void write_root_dir(FILE *file, boot_sector_t boot, dir_entry_t *dir);

// Function declarations for file operations
void create_file(fat12_t *fs, char *path);

void read_file(fat12_t *fs, char *path);

void write_file(fat12_t *fs, char *path, void *data, size_t size);

void delete_file(fat12_t *fs, char *path);

// Function declarations for directory operations
dir_entry_t *fetch_dir_entry(fat12_t *fs, char *path);

void create_dir(fat12_t *fs, char *dir_name, dir_entry_t *parent_dir);

void delete_dir(fat12_t *fs, char *path);

void list_dir(fat12_t *fs, char *path);

//read dir
dir_entry_t *read_directory(fat12_t *fs, int address);

// Function declarations for filesystem operations
void create_filesystem(char *filename, size_t size);

fat12_t *read_filesystem(char *filename);


// Function declarations for utility functions

void print_error(char *message);

void print_error_and_exit(char *message);


//parse path
char **parse_path(char *path);

//get file name from path
char *get_file_name(char *path);

void add_entry_to_directory(fat12_t *fs, char *path, char *entry_name, uint8_t attr, void *data, size_t size);
dir_entry_t *read_directory(fat12_t *fs, uint_12 cluster);

uint_12 allocate_cluster(uint_12 *fat, int block_count);

void write_directory(FILE *pFile, boot_sector_t sector, uint_12 cluster, dir_entry_t *dir);

void *read_data_from_file(FILE *file, boot_sector_t boot_sector, uint_12 cluster, size_t size);

#endif // HW_3_FILESYSTEM_H
