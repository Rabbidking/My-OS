// Interrupt Handler Initialization
#pragma once
#pragma pack(push,1)
struct InterruptFrame{
    unsigned eip;
    unsigned cs;
    unsigned eflags;
    unsigned esp; //only used when undergoing
    unsigned ss; //a ring transition
};
#pragma pack(pop)
