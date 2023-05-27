#ifndef PAGE_TABLE_ENTRY_H
#define PAGE_TABLE_ENTRY_H

#include <ctime>

class PageTableEntry
{
public:
    PageTableEntry();

    void markValid();
    void markInvalid();
    void markDirty();
    void markClean();
    void updateTimestamp();
    void updateLastUsed();

    unsigned int getFrameNumber() const;
    bool isValid() const;
    bool isDirty() const;
    std::time_t getTimestamp() const;
    std::time_t getLastUsed() const;

private:
    unsigned int frameNumber;
    bool valid;
    bool dirty;
    std::time_t timestamp;
    std::time_t lastUsed;
};

#endif // PAGE_TABLE_ENTRY_H
