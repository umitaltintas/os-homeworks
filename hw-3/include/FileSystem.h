//
// Created by Codefirst on 6.06.2023.
//

#ifndef HW_3_FILESYSTEM_H
#define HW_3_FILESYSTEM_H

#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <vector>

constexpr int NUM_BLOCKS = 4096; // 2^12 possible addresses



constexpr int BITS_PER_WORD = sizeof(uint32_t) * CHAR_BIT;             // 32
constexpr int BITMAP_WORD_COUNT = NUM_BLOCKS / BITS_PER_WORD;          // 128
constexpr int BITMAP_BYTE_SIZE = sizeof(uint32_t) * BITMAP_WORD_COUNT; // 512

constexpr int FAT_TABLE_BYTE_SIZE = (NUM_BLOCKS * 12) / CHAR_BIT;            // 6 * 1024
constexpr int FAT_TABLE_WORD_COUNT = FAT_TABLE_BYTE_SIZE / sizeof(uint32_t); // 6 * 1024
constexpr int NUM_ROOT_BLOCKS = 14;
constexpr int YEAR_START = 1900;

constexpr int DIRECTORY_ATTRIBUTE = 0;
constexpr int FILE_ATTRIBUTE = 1;

#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b) ((b) % BITS_PER_WORD)

// Rest of the constants, structs, enums and defines here...
struct SuperBlock
{
    uint32_t blockSize;
    uint32_t blockCount;
    uint32_t freeBitmapStart;
    uint32_t freeBlockSize;
    uint32_t numFreeBlocks;
    uint32_t fatTableStart;
    uint32_t fatTableBlockSize;
    uint32_t rootStart;
    uint32_t rootSize;
    uint32_t dataStart;
    uint32_t dataSize;
}
__attribute__((packed));

struct DirectoryEntry
{
    char name[20];         // 20 bytes
    uint16_t address;      // 2 bytes FAT-12 (used only least significant 12 bits)
    uint8_t attribute;     // 1 byte
    uint8_t size[3];       // 3 bytes : max 16 MB size
    uint8_t time[3];       // 3 bytes : HMS
    uint8_t date[3];       // 3 bytes : DMY (Y: 1900+)
} __attribute__((packed)); // required for struct to be exactly 32 bytes

struct Uint12
{
    uint16_t value : 12;
};

class BitmapManager
{
public:
    void init(int numBlocks);

    void setBitInFreeBitmap(int bitIndex);

    void clearBitInFreeBitmap(int bitIndex);

    bool isBitSetInFreeBitmap(int bitIndex) const;

    void printFreeBitmap() const;

    const std::vector<uint32_t> &getFreeBitmap() const { return freeBitmap; }

    void setFreeBitmap(std::vector<uint32_t> &&vector);

private:
    std::vector<uint32_t> freeBitmap;
};

class DiskWriter
{
public:
    DiskWriter(std::string filename);

    void writeSuperBlockToDisk(const SuperBlock &superBlock);

    void writeFreeBitmapToDisk(const std::vector<uint32_t> &freeBitmap, const SuperBlock &superBlock);

    void writeFatTableToDisk(const std::vector<Uint12> &fatTable, const SuperBlock &superBlock);

private:
    std::string filename;
    std::fstream fileStream;

    void writeZeroBytes(int bytes);
};

class FatTableManager
{
public:
    std::vector<Uint12> getFatTable() const;

    void init(int numBlocks);

    void setFatTable(std::vector<Uint12> &&vector);

private:
    std::vector<Uint12> fatTable;
};

class BlockManager
{
public:
    BlockManager(BitmapManager &bitmapManager, FatTableManager &fatTableManager);
    int allocateBlock();
    void deallocateBlock(int blockNumber);
    int getAllocatedBlockCount() const;

private:
    BitmapManager &bitmapManager;
    FatTableManager &fatTableManager;
};

class FileSystem
{
public:
    FileSystem(std::string filename, int blockSizeKB);
    FileSystem(std::string filename);
    void dir(const std::string &path);
    void mkdir(const std::string &path);
    void rmdir(const std::string &path);
    void dumpe2fs();
    void write(const std::string &path, const std::string &linuxFile);
    void read(const std::string &path, const std::string &linuxFile);
    void del(const std::string &path);
    ~FileSystem();

    void create();

private:
    std::string filename;
    int blockSizeKB;
    int numBlocks;
    std::fstream fileStream;
    SuperBlock superBlock;
    BitmapManager bitmapManager;
    DiskWriter diskWriter;
    FatTableManager fatTableManager;


    void tokenizePath(const std::string &path, std::vector<std::string> &tokens);

};

#endif // HW_3_FILESYSTEM_H
