#pragma once
#pragma pack(push,1)
struct GDTEntry{
    // Create an entry in our Global Descriptor Table
    unsigned short limitLow;
    unsigned char base0, base1, base2;
    unsigned short flagsAndLimitHigh;
    unsigned char base3;
};
#pragma pack(pop)
