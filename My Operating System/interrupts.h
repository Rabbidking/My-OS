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

#pragma pack(push,1)
struct GDTEntry{
    // Create an entry in our Global Descriptor Table
    unsigned short limitLow;
    unsigned char base0, base1, base2;
    unsigned short flagsAndLimitHigh;
    unsigned char base3;
};
#pragma pack(pop)

#pragma pack(push,1)
struct LGDT{
    // Load Global Descriptor Table
    unsigned short size;
    struct GDTEntry* addr;
};
#pragma pack(pop)

#pragma pack(push,1)
struct InterruptFrame{
    unsigned eip;
    unsigned cs;
    unsigned eflags;
    unsigned esp; //only used when undergoing
    unsigned ss; //a ring transition
};
#pragma pack(pop)

#pragma pack(push,1)
struct LIDT{
    //Load Interrupt Descriptor Table
    unsigned short size;
    struct IDTEntry* addr;
};
#pragma pack(pop)

void unknownInterrupt(struct InterruptFrame* fr);   //0 1, and 6
void unknownInterruptWithCode(struct InterruptFrame* fr, unsigned code); //13 and 14
void haltForever(void);
void interrupt_init();
void divide_by_zero(struct InterruptFrame* fr);
void debug_trap(struct InterruptFrame* fr);
void bad_opcode(struct InterruptFrame* fr);
void stack_fault(struct InterruptFrame* fr, unsigned code);
void page_fault(struct InterruptFrame* fr, unsigned code);
void table(int i, void* func);
int exec(const char* filename);
