#include "FileSystem.h"
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define FAT_SIZE (BLOCK_COUNT * sizeof(uint_12))


dir_entry_t *fetch_parent_dir_entry(fat12_t *fs, char *path);


char *get_parent_path(char *path);

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
    int root_dir_start = sizeof(boot_sector_t) + boot.block_count * sizeof(uint_12);

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
    fat[0].value = 0x0FFF;
    // Write FAT to disk
    write_fat(file, boot, fat);

    // Write root directory to disk
    dir_entry_t *root_dir = (dir_entry_t *) (calloc(block_size, sizeof(dir_entry_t)));
    memset(root_dir, 0x000, block_size);
    root_dir->filename[0] = '/';
    root_dir->date = 0x0000;
    root_dir->time = 0x0000;
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
    char *path_copy = strdup(path);
    char **tokens = (char **) (malloc(sizeof(char *) * 10));
    char *token = strtok(path_copy, "/");
    int i = 0;
    while (token != NULL) {
        tokens[i] = strdup(token);
        token = strtok(NULL, "/");
        i++;
    }
    tokens[i] = NULL;
    free(path_copy);
    return tokens;
}


dir_entry_t *fetch_dir_entry(fat12_t *fs, char *path) {
    if (strcmp(path, "/") == 0) {
        return fs->root_dir;
    }

    char **tokens = parse_path(path);
    int i = 0;
    dir_entry_t *current_dir = fs->root_dir;
    dir_entry_t *dir_entry = nullptr;

    while (tokens[i] != nullptr) {
        bool found = false;
        for (int j = 0; j < fs->boot_sector.root_dir_entries_count; j++) {
            if (current_dir[j].filename[0] != '\0' &&
                current_dir[j].attr == ATTR_DIRECTORY &&
                strncmp(tokens[i], (char *) current_dir[j].filename, 8) == 0) {
                dir_entry = &current_dir[j];
                current_dir = read_directory(fs, dir_entry->starting_cluster);
                found = true;
                break;
            }
        }
        if (!found) {
            current_dir = read_directory(fs, current_dir->starting_cluster);
            create_dir(fs, tokens[i], current_dir);
            dir_entry = &current_dir[1];
            current_dir = read_directory(fs,
                                         dir_entry->starting_cluster);
        }
        i++;
    }
    return dir_entry;
}


dir_entry_t *fetch_dir_entry_without_creation(fat12_t *fs, char *path) {

    if (strcmp(path, "/") == 0) {
        return fs->root_dir;
    }

    char **tokens = parse_path(path);
    int i = 0;
    dir_entry_t *current_dir = fs->root_dir;
    dir_entry_t *dir_entry = nullptr;

    while (tokens[i] != nullptr) {
        bool found = false;
        for (int j = 0; j < fs->boot_sector.root_dir_entries_count; j++) {
            if (current_dir[j].filename[0] != '\0' &&
                current_dir[j].attr == ATTR_DIRECTORY &&
                strncmp(tokens[i], (char *) current_dir[j].filename, 8) == 0) {
                dir_entry = &current_dir[j];
                current_dir = read_directory(fs, dir_entry->starting_cluster);
                found = true;
                break;
            }
                //if it is a file and last token
            else if (current_dir[j].filename[0] != '\0' &&
                     current_dir[j].attr == ATTR_FILE &&
                     strncmp(tokens[i], (char *) current_dir[j].filename, 8) == 0 &&
                     tokens[i + 1] == nullptr) {
                dir_entry = &current_dir[j];
                found = true;
                break;
            }
        }
        if (!found) {
            return nullptr;
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
        } else if (strncmp((char *) parent_dir[i].filename, dir_name, 8) == 0) {
            print_error_and_exit("Directory with the same name already exists");
        }
    }

    // Check if we found an empty entry
    if (i == fs->boot_sector.root_dir_entries_count) {
        print_error_and_exit("Directory is full");
    }

    // Create the directory entry
    dir_entry_t *dir;
    dir = (dir_entry_t *) (malloc(sizeof(dir_entry_t)));
    // Set the filename to all 0s
    memset(dir, 0, sizeof(dir_entry_t));
    //copy name
    strncpy((char *) dir->filename, dir_name, 8);
    dir->attr = ATTR_DIRECTORY;
    // Write the directory entry to the parent directory
    uint_12 cluster = allocate_cluster(fs->fat, fs->boot_sector.block_count);
    dir->starting_cluster = cluster;
    parent_dir[i] = *dir;
    write_directory(fs->file, fs->boot_sector, parent_dir->starting_cluster, parent_dir);
    write_directory(fs->file, fs->boot_sector, dir->starting_cluster, dir);
    // Update FAT
    write_fat(fs->file, fs->boot_sector, fs->fat);
    std::cout << "Directory created successfully.\n";
}


dir_entry_t *read_directory(fat12_t *fs, uint_12 cluster) {
    // Calculate directory start location
    int dir_start = sizeof(boot_sector_t) + FAT_SIZE +
                    cluster.value * fs->boot_sector.block_size;

    // Allocate space for directory
    dir_entry_t *dir = (dir_entry_t *) (malloc(fs->boot_sector.root_dir_entries_count * sizeof(dir_entry_t)));

    // Position the file pointer to the beginning of the directory
    fseek(fs->file, dir_start, SEEK_SET);

    // Read directory into memory
    fread(dir, sizeof(dir_entry_t) * fs->boot_sector.root_dir_entries_count, 1, fs->file);

    return dir;
}


void list_dir(fat12_t *fs, char *path) {
    // Fetch directory entry
    dir_entry_t *dir_entry = fetch_dir_entry_without_creation(fs, path);

    dir_entry_t *current_dir = read_directory(fs, dir_entry->starting_cluster);
    if (!current_dir) {
        print_error_and_exit("Directory not found");
    }


    for (int i = 1; i < fs->boot_sector.root_dir_entries_count; i++) {
        if (current_dir[i].filename[0] != '\0') {
            std::cout << current_dir[i].filename << "\n";
        }
    }
}

void delete_dir(fat12_t *fs, char *path) {
    dir_entry_t *dir_entry = fetch_dir_entry_without_creation(fs, path);
    if (!dir_entry) {
        print_error_and_exit("Directory not found");
    }
    if ((dir_entry->attr & ATTR_DIRECTORY) == 0) {
        print_error_and_exit("Not a directory");
    }
    dir_entry_t *current_dir = read_directory(fs, dir_entry->starting_cluster);
    for (int i = 1; i < fs->boot_sector.root_dir_entries_count; i++) {
        if (current_dir[i].filename[0] != '\0') {
            print_error_and_exit("Directory is not empty");
        }
    }
    current_dir->filename[0] = '\0';
    write_directory(fs->file, fs->boot_sector, current_dir->starting_cluster, current_dir);

    // get parent dir_entry and remove directory entry
    dir_entry_t *parent_dir_entry = fetch_parent_dir_entry(fs, path);
    remove_entry_from_dir(fs, parent_dir_entry, dir_entry);

    uint_12 cluster = dir_entry->starting_cluster;
    while (cluster.value != 0xFFF) {
        uint_12 next_cluster = fs->fat[cluster.value];
        fs->fat[cluster.value].value = 0;
        cluster = next_cluster;
    }
    write_fat(fs->file, fs->boot_sector, fs->fat);
    write_root_dir(fs->file, fs->boot_sector, fs->root_dir);
}

void delete_file(fat12_t *fs, char *path) {
    dir_entry_t *dir_entry = fetch_dir_entry_without_creation(fs, path);
    if (!dir_entry) {
        print_error_and_exit("File not found");
    }
    if (dir_entry->attr != ATTR_FILE) {
        print_error_and_exit("Not a file");
    }

    // get parent dir_entry and remove file entry
    dir_entry_t *parent_dir_entry = fetch_parent_dir_entry(fs, path);
    remove_entry_from_dir(fs, parent_dir_entry, dir_entry);

    uint_12 cluster = dir_entry->starting_cluster;
    while (cluster.value != 0xFFF) {
        uint_12 next_cluster = fs->fat[cluster.value];
        fs->fat[cluster.value].value = 0;
        cluster = next_cluster;
    }
    write_fat(fs->file, fs->boot_sector, fs->fat);
    write_directory(fs->file, fs->boot_sector, dir_entry->starting_cluster, dir_entry);
}

void remove_entry_from_dir(fat12_t *fs, dir_entry_t *entry, dir_entry_t *entry1) {
    dir_entry_t *current_dir = read_directory(fs, entry->starting_cluster);
    for (int i = 1; i < fs->boot_sector.root_dir_entries_count; i++) {
        if (current_dir[i].filename[0] != '\0') {
            if (strcmp((char *) current_dir[i].filename, (char *) entry1->filename) == 0) {
                current_dir[i].filename[0] = '\0';
                write_directory(fs->file, fs->boot_sector, current_dir->starting_cluster, current_dir);
                return;
            }
        }
    }
    print_error_and_exit("File not found");
}

dir_entry_t *fetch_parent_dir_entry(fat12_t *fs, char *path) {
    char *parent_path = get_parent_path(path);
    dir_entry_t *parent_dir_entry = fetch_dir_entry_without_creation(fs, parent_path);
    free(parent_path);
    return parent_dir_entry;

}

char *get_parent_path(char *path) {
    char *parent_path = strdup(path);
    char *last_slash = strrchr(parent_path, '/');
    if (last_slash) {
        *last_slash = '\0';
    } else {
        print_error_and_exit("Invalid path");
    }
    return parent_path;
}


void read_file(fat12_t *fs, char *path, char *output_path) {
    // Fetch file entry
    dir_entry_t *dir_entry = fetch_dir_entry_without_creation(fs, path);
    if (!dir_entry) {
        print_error_and_exit("File not found");
    }

    // Check if the entry is a file
    if ((dir_entry->attr & ATTR_FILE) != 0) {
        print_error_and_exit("Not a file");
    }

    // Read file data
    void *data = read_data_from_file(fs->file, fs->boot_sector, dir_entry->starting_cluster, dir_entry->file_size);

    // Write data to output file
    FILE *output_file = fopen(output_path, "wb");
    fwrite(data, dir_entry->file_size, 1, output_file);
    fclose(output_file);
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
            uint_12 new_cluster;
            new_cluster.value = i;
            return new_cluster;
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


void write_boot_sector(FILE *file, boot_sector_t boot) {
    fseek(file, 0, SEEK_SET);
    fwrite(&boot, sizeof(boot_sector_t), 1, file);
}


void write_file(fat12_t *fs, char *path, void *data, size_t size) {

    // Write file data to disk
    //copy path without file name
    char *file_name = get_file_name(path);
    *(file_name - 1) = '\0';

    add_entry_to_directory(fs, path, file_name, ATTR_FILE, data, size);
    // Update file size

    // Write updated FAT and root directory to disk
    write_fat(fs->file, fs->boot_sector, fs->fat);
    write_root_dir(fs->file, fs->boot_sector, fs->root_dir);
}

void add_entry_to_directory(fat12_t *fs, char *path, char *entry_name, uint8_t attr, void *data, size_t size) {
    // Fetch parent directory entry
    dir_entry_t *parent_dir_entry = fetch_dir_entry(fs, path);
    if (!parent_dir_entry) {
        print_error_and_exit("Directory not found");
    }

    // Read parent directory
    dir_entry_t *parent_dir = read_directory(fs, parent_dir_entry->starting_cluster);

    // Check if the entry already exists
    for (int i = 0; i < fs->boot_sector.root_dir_entries_count; i++) {
        if (strcmp((char *) parent_dir[i].filename, entry_name) == 0) {
            print_error_and_exit("File already exists");
        }
    }

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

    // Allocate a new cluster for the file
    uint_12 cluster = allocate_cluster(fs->fat, fs->boot_sector.block_count);

    // Write the data to the file
    write_data_to_file(fs->file, fs->boot_sector, cluster, data, size);

    // Create the directory entry
    dir_entry_t entry;
    memset(&entry, 0, sizeof(dir_entry_t));
    strncpy((char *) entry.filename, entry_name, sizeof(entry.filename));
    entry.attr = attr;
    entry.file_size = size;
    entry.starting_cluster = cluster;

    // Write the directory entry to the parent directory
    parent_dir[i] = entry;
    write_directory(fs->file, fs->boot_sector, parent_dir_entry->starting_cluster, parent_dir);

    // Update FAT
    write_fat(fs->file, fs->boot_sector, fs->fat);

    std::cout << "Entry added successfully.\n";
}

void dumpe2fs(fat12_t *fs) {
    std::cout << "Block count: " << fs->boot_sector.block_count << "\n";
    std::cout << "Block size: " << fs->boot_sector.block_size << "\n";
    std::cout << "Root directory entries count: " << fs->boot_sector.root_dir_entries_count << "\n";


    std::cout << "Root directory:\n";
    dir_entry_t *root_dir = read_directory(fs, {0});
    for (int i = 0; i < fs->boot_sector.root_dir_entries_count; i++) {
        std::cout << i << ": " << root_dir[i].filename << "\n";
    }

    std::cout << "Files:\n";
    for (int i = 0; i < fs->boot_sector.root_dir_entries_count; i++) {
        if (root_dir[i].filename[0] != '\0') {
            std::cout << i << ": " << root_dir[i].filename << "\n";
        }
    }


}
