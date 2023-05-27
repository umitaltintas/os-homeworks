#include "page_table_entry.h"

PageTableEntry::PageTableEntry() : frameNumber(0), valid(false), dirty(false), timestamp(0), lastUsed(0)
{
    // Initial values for a new page table entry
}

void PageTableEntry::markValid()
{
    valid = true;
}

void PageTableEntry::markInvalid()
{
    valid = false;
}

void PageTableEntry::markDirty()
{
    dirty = true;
}

void PageTableEntry::markClean()
{
    dirty = false;
}

void PageTableEntry::updateTimestamp()
{
    timestamp = std::time(0);
}

void PageTableEntry::updateLastUsed()
{
    lastUsed = std::time(0);
}

unsigned int PageTableEntry::getFrameNumber() const
{
    return frameNumber;
}

bool PageTableEntry::isValid() const
{
    return valid;
}

bool PageTableEntry::isDirty() const
{
    return dirty;
}

std::time_t PageTableEntry::getTimestamp() const
{
    return timestamp;
}

std::time_t PageTableEntry::getLastUsed() const
{
    return lastUsed;
}