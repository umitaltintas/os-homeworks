#include "FileSystem.h"
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define FAT_SIZE (BLOCK_COUNT * sizeof(uint_12))


uint_12 *read_fat(FILE *file, boot_sector_t boot) {
    // Calculate FAT location
    int fat_start = sizeof(boot_sector_t);


    // Allocate space for FAT
    uint_12 *fat = (uint_12 *) (malloc(boot.block_count * sizeof(uint_12)));

    // Position the file pointer to the beginning of the FAT
    fseek(file, fat_start, SEEK_SET);

    // Read FAT into memory
    fread(fat, boot.block_count * sizeof(uint_12), 1, file);

    return fat;
}

void write_fat(FILE *file, boot_sector_t boot, uint_12 *fat) {
    // Calculate FAT location
    int fat_start = sizeof(boot_sector_t);

    // Position the file pointer to the beginning of the FAT
    fseek(file, fat_start, SEEK_SET);

    // Write FAT to disk
    fwrite(fat, boot.block_count * sizeof(uint_12), 1, file);
}

dir_entry_t *read_root_dir(FILE *file, boot_sector_t boot) {
    // Calculate root directory location
    int root_dir_start = sizeof(boot_sector_t) + boot.block_count * sizeof(uint_12);


    // Allocate space for root directory
    dir_entry_t *root_dir = (dir_entry_t *) (malloc(boot.root_dir_entries_count * sizeof(dir_entry_t)));

    // Position the file pointer to the beginning of the root directory
    fseek(file, root_dir_start, SEEK_SET);

    // Read root directory into memory
    fread(root_dir, sizeof(dir_entry_t) * boot.root_dir_entries_count, 1, file);

    return root_dir;
}


void write_root_dir(FILE *file, boot_sector_t boot, dir_entry_t *dir) {
    // Calculate root directory location
    int root_dir_start = sizeof(boot_sector_t) + boot.block_count * boot.block_size;

    // Position the file pointer to the beginning of the root directory
    fseek(file, root_dir_start, SEEK_SET);

    // Write root directory to disk
    fwrite(dir, boot.root_dir_entries_count * sizeof(dir_entry_t), 1, file);
}

void create_filesystem(char *filename, size_t block_size) {
    FILE *file = fopen(filename, "w");
    if (file == nullptr) {
        std::cerr << "Error: Unable to open file " << filename << "\n";
        exit(1);
    }

    // Calculate number of blocks
    size_t block_count = BLOCK_COUNT;



    // Initialize boot sector
    boot_sector_t boot;
    boot.block_size = block_size;
    boot.block_count = block_count;
    boot.root_dir_entries_count = ROOT_DIR_ENTRIES_COUNT;

    // Write boot sector to disk
    write_boot_sector(file, boot);



    // Initialize FAT
    uint_12 *fat = (uint_12 *) (calloc(block_count * block_size, sizeof(uint_12)));
    memset(fat, 0x000, block_count * block_size);
    // Write FAT to disk
    write_fat(file, boot, fat);

    // Write root directory to disk
    dir_entry_t *root_dir = (dir_entry_t *) (calloc(block_size, sizeof(dir_entry_t)));
    memset(root_dir, 0x000, block_size);
    //set as directory
    root_dir->attr = ATTR_DIRECTORY;
    write_root_dir(file, boot, root_dir);

    fclose(file);

    std::cout << "File system created successfully.\n";
}


fat12_t *read_filesystem(char *filename) {
    FILE *file = fopen(filename, "r+");
    if (file == nullptr) {
        std::cerr << "Error: Unable to open file " << filename << "\n";
        exit(1);
    }

    // Read boot sector
    boot_sector_t boot;
    fread(&boot, sizeof(boot_sector_t), 1, file);

    // Read FAT
    uint_12 *fat = read_fat(file, boot);

    // Read root directory
    dir_entry_t *root_dir = read_root_dir(file, boot);

    // Initialize file system
    fat12_t *fs = (fat12_t *) (malloc(sizeof(fat12_t)));
    fs->file = file;
    fs->boot_sector = boot;
    fs->fat = fat;
    fs->root_dir = root_dir;
    return fs;
}

void print_error(std::string message) {
    std::cerr << "Error: " << message << "\n";
}

void print_error_and_exit(std::string message) {
    print_error(message);
    exit(1);
}

char **parse_path(char *path) {
    char **tokens = (char **) (malloc(sizeof(char *) * 10));
    char *token = strtok(path, "/");
    int i = 0;
    while (token != nullptr) {
        tokens[i] = token;
        token = strtok(nullptr, "/");
        i++;
    }
    tokens[i] = nullptr;
    return tokens;
}


dir_entry_t *fetch_dir_entry(fat12_t *fs, char *path) {
    // Handle root directory
    if (strcmp(path, "/") == 0) {
        return fs->root_dir;
    }

    // Parse the path to extract the directory name and parent path
    char **tokens = parse_path(path);
    int i = 0;
    dir_entry_t *current_dir = fs->root_dir;
    dir_entry_t *dir_entry = nullptr;

    // Traverse the path to the parent directory
    while (tokens[i] != nullptr) {
        bool found = false;
        for (int j = 0; j < fs->boot_sector.root_dir_entries_count; j++) {
            if (current_dir[j].filename[0] != '\0' &&
                current_dir[j].attr == ATTR_DIRECTORY &&
                strncmp(tokens[i], (char *) current_dir[j].filename, 8) == 0) {
                // Found the directory, move to the next level
                dir_entry = &current_dir[j];
                current_dir = read_directory(fs, dir_entry->starting_cluster);
                found = true;
                break;
            }
        }
        if (!found) {
            // Directory not found, create it
            create_dir(fs, tokens[i], current_dir);
            current_dir = read_directory(fs, current_dir->starting_cluster);
            dir_entry = &current_dir[0];
        }
        i++;
    }
    return dir_entry;
}

void create_dir(fat12_t *fs, char *dir_name, dir_entry_t *parent_dir) {
    // Find an empty entry in the parent directory
    int i;
    for (i = 0; i < fs->boot_sector.root_dir_entries_count; i++) {
        if (parent_dir[i].filename[0] == '\0') {
            break;
        }
    }

    // Check if we found an empty entry
    if (i == fs->boot_sector.root_dir_entries_count) {
        print_error_and_exit("Directory is full");
    }

    // Create the directory entry
    dir_entry_t dir;
    memset(&dir, 0, sizeof(dir_entry_t));
    strncpy((char *) dir.filename, dir_name, sizeof(dir.filename));
    dir.attr = ATTR_DIRECTORY;

    // Write the directory entry to the parent directory
    parent_dir[i] = dir;
    uint_12 cluster = allocate_cluster(fs->fat, fs->boot_sector.block_count);
    parent_dir[i].starting_cluster = cluster;
    write_directory(fs->file, fs->boot_sector, parent_dir->starting_cluster, parent_dir);

    // Update FAT

    write_fat(fs->file, fs->boot_sector, fs->fat);

    std::cout << "Directory created successfully.\n";
}


dir_entry_t *read_directory(fat12_t *fs, uint_12 cluster) {
    // Calculate directory start location
    int dir_start = sizeof(boot_sector_t) + fs->boot_sector.block_count * sizeof(uint_12) +
                    cluster.value * fs->boot_sector.block_size;

    // Allocate space for directory
    dir_entry_t *dir = static_cast<dir_entry_t *>(malloc(fs->boot_sector.root_dir_entries_count * sizeof(dir_entry_t)));

    // Position the file pointer to the beginning of the directory
    fseek(fs->file, dir_start, SEEK_SET);

    // Read directory into memory
    fread(dir, sizeof(dir_entry_t) * fs->boot_sector.root_dir_entries_count, 1, fs->file);

    return dir;
}

void list_dir(fat12_t *fs, char *path) {
    // Fetch directory entry
    dir_entry_t *dir_entry = fetch_dir_entry(fs, path);
    if (!dir_entry) {
        print_error_and_exit("Directory not found");
    }

    // Read directory
    dir_entry_t *dir = read_directory(fs, dir_entry->starting_cluster);

    // List directory content
    for (int i = 0; i < fs->boot_sector.root_dir_entries_count; i++) {
        if (dir[i].filename[0] != '\0') {
            printf("%s\n", dir[i].filename);
        }
    }
}

void delete_dir(fat12_t *fs, char *path) {
    // Fetch directory entry
    dir_entry_t *dir_entry = fetch_dir_entry(fs, path);
    if (!dir_entry) {
        print_error_and_exit("Directory not found");
    }

    // Remove directory entry
    dir_entry->filename[0] = '\0';

    // Free blocks in FAT
    uint_12 cluster = dir_entry->starting_cluster;
    while (cluster.value != 0xFFF) {
        uint_12 next_cluster = fs->fat[cluster.value];
        cluster.value = 0;
        cluster = next_cluster;
    }

    // Write FAT to disk
    write_fat(fs->file, fs->boot_sector, fs->fat);
}

void delete_file(fat12_t *fs, char *path) {
    // Similar to delete_dir, but also checks if the entry is a file
    dir_entry_t *dir_entry = fetch_dir_entry(fs, path);
    if (!dir_entry) {
        print_error_and_exit("File not found");
    }

    if ((dir_entry->attr & ATTR_DIRECTORY) != 0) {
        print_error_and_exit("Not a file");
    }

    dir_entry->filename[0] = '\0';
    uint_12 cluster = dir_entry->starting_cluster;
    while (cluster.value != 0xFFF) {
        uint_12 next_cluster = fs->fat[cluster.value];
        cluster.value = 0;
        cluster = next_cluster;
    }
    write_fat(fs->file, fs->boot_sector, fs->fat);
}


void read_file(fat12_t *fs, char *path) {
    // Fetch file entry
    dir_entry_t *dir_entry = fetch_dir_entry(fs, path);
    if (!dir_entry) {
        print_error_and_exit("File not found");
    }

    // Check if the entry is a file
    if ((dir_entry->attr & ATTR_DIRECTORY) != 0) {
        print_error_and_exit("Not a file");
    }

    // Read file data
    void *data = read_data_from_file(fs->file, fs->boot_sector, dir_entry->starting_cluster, dir_entry->file_size);

    // Print data
    printf("%s\n", (char *) (data));
}

char *get_file_name(char *path) {
    char *file_name = strrchr(path, '/');
    if (!file_name) {
        return path;
    } else {
        return file_name + 1;
    }
}

uint_12 allocate_cluster(uint_12 *fat, int block_count) {
    for (int i = 0; i < block_count; i++) {
        if (fat[i].value == 0) {
            fat[i].value = 0xFFF;  // Mark as allocated
            return fat[i];
        }
    }
    print_error_and_exit("No space left on device");
    return {};  // To silence compiler warning
}

void write_directory(FILE *file, boot_sector_t boot_sector, uint_12 cluster, dir_entry_t *dir) {
    // Calculate directory start location
    int dir_start =
            sizeof(boot_sector_t) + boot_sector.block_count * sizeof(uint_12) + cluster.value * boot_sector.block_size;

    // Position the file pointer to the beginning of the directory
    fseek(file, dir_start, SEEK_SET);

    // Write directory into memory
    fwrite(dir, sizeof(dir_entry_t) * boot_sector.root_dir_entries_count, 1, file);
}

void write_data_to_file(FILE *file, boot_sector_t boot_sector, uint_12 cluster, void *data, size_t size) {
    // Calculate file start location
    int file_start =
            sizeof(boot_sector_t) + boot_sector.block_count * sizeof(uint_12) + cluster.value * boot_sector.block_size;

    // Position the file pointer to the beginning of the file
    fseek(file, file_start, SEEK_SET);

    // Write data to the file
    fwrite(data, size, 1, file);
}

void *read_data_from_file(FILE *file, boot_sector_t boot_sector, uint_12 cluster, size_t size) {
    // Calculate file start location
    int file_start =
            sizeof(boot_sector_t) + boot_sector.block_count * sizeof(uint_12) + cluster.value * boot_sector.block_size;

    // Allocate memory for file data
    void *data = malloc(size);

    // Position the file pointer to the beginning of the file
    fseek(file, file_start, SEEK_SET);

    // Read file data into memory
    fread(data, size, 1, file);

    return data;
}


void create_file(fat12_t *fs, char *path) {
    // Similar to write_file, but doesn't write any data
    char *file_name = get_file_name(path);
    dir_entry_t *parent_dir_entry = fetch_dir_entry(fs, path);
    if (!parent_dir_entry) {
        print_error_and_exit("Directory not found");
    }

    dir_entry_t *dir = read_directory(fs, parent_dir_entry->starting_cluster);
    for (int i = 0; i < fs->boot_sector.root_dir_entries_count; i++) {
        if (dir[i].filename[0] == '\0') {
            strncpy((char *) dir[i].filename, file_name, sizeof(dir[i].filename));
            dir[i].attr = ATTR_ARCHIVE;
            dir[i].starting_cluster = allocate_cluster(fs->fat, fs->boot_sector.block_count);
            dir[i].file_size = 0;
            break;
        }
    }

    write_directory(fs->file, fs->boot_sector, parent_dir_entry->starting_cluster, dir);
    write_fat(fs->file, fs->boot_sector, fs->fat);
}


void write_boot_sector(FILE *file, boot_sector_t boot) {
    fseek(file, 0, SEEK_SET);
    fwrite(&boot, sizeof(boot_sector_t), 1, file);
}


void write_file(fat12_t *fs, char *file_name, void *data, size_t size) {

    FILE *file = fs->file;

    boot_sector_t boot_sector;


    // Allocate a new cluster for the file's data
    uint_12 cluster = allocate_cluster(fs->fat, boot_sector.block_count);

    // Write the data to the newly allocated cluster
    write_data_to_file(file, boot_sector, cluster, data, size);

    // Create a new directory entry for the file
    dir_entry_t dir_entry;
    strcpy((char *) dir_entry.filename, file_name);
    dir_entry.starting_cluster = cluster;
    dir_entry.file_size = size;

    // Find an empty spot in the root directory for the new entry
    dir_entry_t *root_dir = read_root_dir(file, boot_sector);
    for (int i = 0; i < boot_sector.root_dir_entries_count; i++) {
        if (root_dir[i].starting_cluster.value == 0) {  // Empty directory entry
            root_dir[i] = dir_entry;
            break;
        }
    }

    // Write the updated root directory back to the file system
    write_root_dir(file, boot_sector, root_dir);


}
