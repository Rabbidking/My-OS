#pragma once
#pragma pack(push,1)
struct IDTEntry{
    // Create an entry in our Interrupt Descriptor Table
    unsigned short addrLow;
    unsigned short selector; //code segment
    unsigned char zero; //must be zero
    unsigned char flags; //0x8e for interrupt handler
    unsigned short addrHigh;
};
#pragma pack(pop)
