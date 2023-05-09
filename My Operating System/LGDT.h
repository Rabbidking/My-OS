// Loading the Global Descriptor Table
#pragma once
#pragma pack(push,1)
struct LGDT{
    // Load Global Descriptor Table
    unsigned short size;
    struct GDTEntry* addr;
};
#pragma pack(pop)
