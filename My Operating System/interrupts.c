#include "utils.h"
#include "interrupts.h"

//choose a value that's big enough, but not too big...
unsigned char kernelStack[65536];
unsigned ring0StackInfo[] = {
    0,
    (unsigned)(kernelStack+sizeof(kernelStack)),
    1<<3
};

// Interrupt handlers for default/unknown cases.
__attribute__((__interrupt__))
void unknownInterrupt(struct InterruptFrame* fr){
    kprintf("\nFatal exception at eip=%x\n",fr->eip);
    haltForever();
}

__attribute__((__interrupt__))
void unknownInterruptWithCode(struct InterruptFrame* fr, unsigned code){
    kprintf("Fatal exception: Code=%x eip=%x\n",code,fr->eip);
    haltForever();
}

// Creates the first entry in our Global Descriptor Table (must be all zeroes)
struct GDTEntry gdt[] = {
    { 0,0,0,0,0,0 }, //zeros
    { 0xffff, 0,0,0, 0xcf92, 0}, //data, ring 0
    { 0xffff, 0,0,0, 0xcf9a, 0}, //code, ring 0
    { 0xffff, 0,0,0, 0xcff2, 0}, //data, ring 3
    { 0xffff, 0,0,0, 0xcffa, 0}, //code, ring 3
    { 0,0,0,0,0,0 }              //task selector
};

// Main interrupt handlers for this lab / future labs.
__attribute__((__interrupt__))
void divide_by_zero(struct InterruptFrame* fr)
{
    kprintf("Fatal error: divide by zero!");
    haltForever();
}

__attribute__((__interrupt__))
void debug_trap(struct InterruptFrame* fr)
{
    kprintf("Fatal exception: debug trap");
    haltForever();
}

__attribute__((__interrupt__))
void bad_opcode(struct InterruptFrame* fr)
{
    kprintf("Fatal exception: bad opcode");
    haltForever();
}

__attribute__((__interrupt__))
void stack_fault(struct InterruptFrame* fr, unsigned code)
{
    kprintf("Fatal exception: Stack fault");
    haltForever();
}

__attribute__((__interrupt__))
void page_fault(struct InterruptFrame* fr, unsigned code)
{
    kprintf("Fatal exception: Page fault");
    haltForever();
}

void haltForever(void){
    while(1){
        asm volatile("hlt" ::: "memory");
    }
}

struct IDTEntry idt[32];
// Utility function to help write the interrupt descriptor table
void table(int i, void* func){
    unsigned x = (unsigned)func;
    idt[i].addrLow = x&0xffff;
    idt[i].selector = 2 << 3;
    idt[i].zero = 0;
    idt[i].flags = 0x8e;
    idt[i].addrHigh = x>>16;
}

// Initialization of the GDT, via the LGDT
void interrupt_init(){

    //Task selector initialization
    unsigned tmp = (unsigned) ring0StackInfo;
    gdt[5].limitLow = sizeof(ring0StackInfo);
    gdt[5].base0 = tmp & 0xff;
    gdt[5].base1 = (tmp>>8) & 0xff;
    gdt[5].base2 = (tmp>>16) & 0xff;
    gdt[5].flagsAndLimitHigh = 0x00e9;
    gdt[5].base3 = (tmp>>24) & 0xff;

    /*
    Time to set up the table.
    * For each of the 32 entries of idt:
        * If this is an interrupt that pushes a code:
        table( num , unknownInterruptWithCode )
        * Else:
        table( num, unknownInterrupt);

        purple = unknownInterrupt, pink = unknownInterruptWithCode
        If we didn't care about specific interrupts, a way to simplify it would be:
        for(i = 0; i < 7; i++)
        {
            table(i, unknownInterrupt());
        }*/

    switch(idt->selector){
        case 0:
            table(0, divide_by_zero);
        case 1:
            table(1, debug_trap);
        case 6:
            table(6, bad_opcode);
        case 13:
            table(13, stack_fault);
        case 14:
            table(14, page_fault);
    }

    // Revised LGDT for exes
    struct LGDT lgdt;
    lgdt.size = sizeof(gdt);
    lgdt.addr = &gdt[0];
    asm volatile( "lgdt [eax]\n"
                  "ltr bx"
                : //no outputs
                : "a"(&lgdt), //put address of gdt in eax
                    "b"( (5<<3) | 3 ) //put task register index in ebx
                : "memory" );

    // Interrupt initialization
    /*struct LIDT temp;
    temp.size = sizeof(idt);
    temp.addr = &idt[0];
    asm volatile("lidt [eax]" : : "a"(&temp) : "memory" );*/
}

int exec(const char* filename)
{
    asm volatile(
        "mov ax,27\n"
        "mov ds,ax\n"
        "mov es,ax\n"
        "mov fs,ax\n"
        "mov gs,ax\n"
        "push 27\n"
        "push 0x800000\n"
        "pushf\n" //push eflags register
        "push 35\n"
        "push 0x400000\n"
        "iret"
        ::: "eax","memory" );
    kprintf("We should never get here!\n");
    haltForever();
    return 0;
}
