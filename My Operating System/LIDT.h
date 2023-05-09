#pragma once
#pragma pack(push,1)
struct LIDT{
    //Load Interrupt Descriptor Table
    unsigned short size;
    struct IDTEntry* addr;
};
#pragma pack(pop)
