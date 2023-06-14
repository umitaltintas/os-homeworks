#include "FileSystem.h"
#include <iostream>
#include <iomanip>
#include <sstream>

void BitmapManager::setBitInFreeBitmap(int bitIndex)
{
    // Logic remains similar to your original class
    if (bitIndex >= freeBitmap.size() * BITS_PER_WORD)
    {
        throw std::out_of_range("Bit index out of range in setBitInFreeBitmap");
    }
    freeBitmap[WORD_OFFSET(bitIndex)] |= ((uint32_t)1 << BIT_OFFSET(bitIndex));
}

void BitmapManager::clearBitInFreeBitmap(int bitIndex)
{
    // Logic remains similar to your original class
    if (bitIndex >= freeBitmap.size() * BITS_PER_WORD)
    {
        throw std::out_of_range("Bit index out of range in clearBitInFreeBitmap");
    }
    freeBitmap[WORD_OFFSET(bitIndex)] &= ~((uint32_t)1 << BIT_OFFSET(bitIndex));
}

bool BitmapManager::isBitSetInFreeBitmap(int bitIndex) const
{
    // Logic remains similar to your original class
    if (bitIndex >= freeBitmap.size() * BITS_PER_WORD)
    {
        throw std::out_of_range("Bit index out of range in isBitSetInFreeBitmap");
    }
    uint32_t bit = freeBitmap[WORD_OFFSET(bitIndex)] & ((uint32_t)1 << BIT_OFFSET(bitIndex));
    return bit != 0;
}

void BitmapManager::printFreeBitmap() const
{
    // Logic remains similar to your original class
    std::cout << "====== BITMAP ======";
    for (int i = 0; i < freeBitmap.size() * BITS_PER_WORD; ++i)
    {
        if (i % 64 == 0)
        {
            std::cout << "\n"
                      << std::setw(5) << i << " :";
        }
        if (i % 16 == 0)
            std::cout << " ";
        std::cout << isBitSetInFreeBitmap(i);
    }
    std::cout << "\n";
}

void BitmapManager::init(int numBlocks)
{
    freeBitmap.resize(BITMAP_WORD_COUNT, 0);
}

void BitmapManager::setFreeBitmap(std::vector<uint32_t> &&vector)
{
}

// DiskWriter

void DiskWriter::writeSuperBlockToDisk(const SuperBlock &superBlock)
{
    fileStream.seekp(0);
    if (!fileStream.write(reinterpret_cast<const char *>(&superBlock), sizeof(superBlock)))
    {
        throw std::runtime_error("Error in writeSuperBlockToDisk: fwrite() failed");
    }

    int remainingBytes = superBlock.blockSize - sizeof(superBlock);
    writeZeroBytes(remainingBytes);
}



void DiskWriter::writeFatTableToDisk(const std::vector<Uint12> &fatTable, const SuperBlock &superBlock)
{
    fileStream.seekp(superBlock.fatTableStart);
    int numBytes = sizeof(Uint12) * NUM_BLOCKS;
    if (!fileStream.write(reinterpret_cast<const char *>(fatTable.data()), numBytes))
    {
        throw std::runtime_error("Error in writeFatTableToDisk: fwrite() failed");
    }
}

void DiskWriter::writeFreeBitmapToDisk(const std::vector<uint32_t> &freeBitmap, const SuperBlock &superBlock)
{
    fileStream.seekp(superBlock.freeBitmapStart);
    int numBytes = sizeof(uint32_t) * BITMAP_WORD_COUNT;
    if (!fileStream.write(reinterpret_cast<const char *>(freeBitmap.data()), numBytes))
    {
        throw std::runtime_error("Error in writeFreeBitmapToDisk: fwrite() failed");
    }
}

void DiskWriter::writeZeroBytes(int bytes)
{
    const size_t BUFFER_SIZE = 4096; // Adjust as needed for your system
    char buffer[BUFFER_SIZE] = {0};  // Initialize buffer with zeros

    while (bytes > 0)
    {
        size_t bytesToWrite = std::min(static_cast<size_t>(bytes), BUFFER_SIZE);
        fileStream.write(buffer, bytesToWrite);
        bytes -= bytesToWrite;
    }
}

DiskWriter::DiskWriter(std::string filename)
{
    this->filename = filename;

    fileStream.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    if (!fileStream.is_open())
    {
        fileStream.open(filename, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
        if (!fileStream.is_open())
        {
            throw std::runtime_error("Could not open file " + this->filename);
        }
    }
}

// FatTableManager
std::vector<Uint12> FatTableManager::getFatTable() const
{
    return fatTable;
}

FileSystem::FileSystem(std::string filename, int blockSizeKB)
    : filename(std::move(filename)), blockSizeKB(blockSizeKB), diskWriter(this->filename)
{
    // Calculate the total number of blocks in the file system.
    numBlocks = (16 * 1024) / blockSizeKB;
}

FileSystem::FileSystem(std::string filename)
    : filename(std::move(filename)), diskWriter(this->filename)
{

    // Open the file in binary mode for read/write.
    fileStream.open(this->filename, std::ios::in | std::ios::out | std::ios::binary);

    // Check if the file stream was opened successfully.
    if (!fileStream.is_open())
    {
        throw std::runtime_error("Failed to open file " + this->filename);
    }

    // Read the super block from the disk.
    fileStream.seekg(0);
    if (!fileStream.read(reinterpret_cast<char *>(&superBlock), sizeof(superBlock)))
    {

        throw std::runtime_error("Failed to read super block from disk");
    }

    // Calculate the total number of blocks in the file system.
    numBlocks = superBlock.blockCount;

    // Read the free block bitmap from the disk.
    fileStream.seekg(superBlock.freeBitmapStart);
    std::vector<uint32_t> freeBitmap(BITMAP_WORD_COUNT);
    if (!fileStream.read(reinterpret_cast<char *>(freeBitmap.data()),
                         sizeof(uint32_t) * BITMAP_WORD_COUNT))
    {
        throw std::runtime_error("Failed to read free block bitmap from disk");
    }
    bitmapManager.setFreeBitmap(std::move(freeBitmap));

    // Read the FAT table from the disk.
    fileStream.seekg(superBlock.fatTableStart);
    std::vector<Uint12> fatTable(numBlocks);
    if (!fileStream.read(reinterpret_cast<char *>(fatTable.data()),
                         sizeof(Uint12) * numBlocks))
    {
        throw std::runtime_error("Failed to read FAT table from disk");
    }
    fatTableManager.setFatTable(std::move(fatTable));

    // Perform other necessary initializations.
}

FileSystem::~FileSystem()
{
    // Perform any necessary cleanup.
}

void FileSystem::create()
{
    superBlock.blockSize = blockSizeKB * 1024;
    superBlock.blockCount = numBlocks;
    superBlock.freeBitmapStart = superBlock.blockSize;
    superBlock.fatTableStart = superBlock.freeBitmapStart + BITMAP_WORD_COUNT * sizeof(uint32_t);
    superBlock.fatTableBlockSize = numBlocks * sizeof(Uint12);
    superBlock.rootStart = superBlock.fatTableStart + superBlock.fatTableBlockSize;
    superBlock.rootSize = superBlock.blockSize;
    superBlock.dataStart = superBlock.rootStart + superBlock.rootSize;
    superBlock.dataSize = superBlock.blockSize * (numBlocks - 1);
    diskWriter.writeSuperBlockToDisk(superBlock);

    // Initialize and write the free block bitmap.
    bitmapManager.init(numBlocks);
    diskWriter.writeFreeBitmapToDisk(bitmapManager.getFreeBitmap(), superBlock);

    // Initialize and write the FAT table.
    fatTableManager.init(numBlocks);
    diskWriter.writeFatTableToDisk(fatTableManager.getFatTable(), superBlock);

    // Write the root directory and any other necessary data.

    // Close the file stream.
    fileStream.close();
}

void FatTableManager::init(int numBlocks)
{
    // Allocate space for the FAT table
    fatTable.resize(numBlocks);
    for (int i = 0; i < numBlocks; i++)
    {
        fatTable[i].value = 0;
    }
}

void FatTableManager::setFatTable(std::vector<Uint12> &&vector)
{
}

void FileSystem::dir(const std::string &path) {
    // Tokenize the path
    std::vector<std::string> pathTokens;
    tokenizePath(path, pathTokens);

    // For each token, find the directory at that level in the hierarchy
    for (const std::string &dirName : pathTokens) {

    }

}
// Tokenizes the path using '/' as a delimiter
void FileSystem::tokenizePath(const std::string &path, std::vector<std::string> &tokens)
{
    std::stringstream ss(path);
    std::string token;

    while (std::getline(ss, token, '/'))
    {
        if (!token.empty())
            tokens.push_back(token);
    }
}

void FileSystem::mkdir(const std::string &path)
{
    // Tokenize the path
    std::vector<std::string> pathTokens;
    tokenizePath(path, pathTokens);

    for (const std::string &dirName : pathTokens)
    {

    }
}

void FileSystem::rmdir(const std::string &path)
{
    std::vector<std::string> pathTokens;
    tokenizePath(path, pathTokens);


}

void FileSystem::dumpe2fs()
{

    std::cout << "Block size: " << superBlock.blockSize << "\n";
    std::cout << "Block count: " << superBlock.blockCount << "\n";

}

void FileSystem::write(const std::string &path, const std::string &linuxFile)
{
    std::ifstream inFile(linuxFile, std::ios::binary);
    if (!inFile)
    {
        throw std::runtime_error("Failed to open file " + linuxFile);
    }


    std::vector<char> buffer((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());


}

void FileSystem::read(const std::string &path, const std::string &linuxFile)
{
    std::ofstream outFile(linuxFile, std::ios::binary);
    if (!outFile)
    {
        throw std::runtime_error("Failed to open file " + linuxFile);
    }


    std::vector<char> buffer;


    outFile.write(buffer.data(), buffer.size());
}

void FileSystem::del(const std::string &path)
{
    // Tokenize the path
    std::vector<std::string> pathTokens;
    tokenizePath(path, pathTokens);


}
