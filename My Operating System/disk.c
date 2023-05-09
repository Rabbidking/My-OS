#pragma once
#include "disk.h"
#include "stdarg.h"

//check hardware's busy bit
int isBusy()
{
    return inb(0x1f7) & 0x80;
}

//Access ports
void outb(unsigned short port, unsigned char value)
{
    asm volatile("out dx,al"
        :
        : "a"(value), "d"(port)
        : "memory"
    );
}

void outw(unsigned short port, unsigned short value)
{
    asm volatile("out dx,ax"
        :
        : "a"(value), "d"(port)
        : "memory"
    );
}

//Read values from ports
//reads byte from given port, returns it
unsigned char inb(unsigned short port)
{
    unsigned value;
    asm volatile("in al,dx" : "=a"(value): "d"(port) );
    return (unsigned char) value;
}

//writes byte to given port
unsigned short inw(unsigned short port)
{
    unsigned value;
    asm volatile("in ax,dx" : "=a"(value): "d"(port) );
    return (unsigned short) value;
}

int disk_read_sector(unsigned sector, void* datablock, int count)
{
    //SETUP CODE
    while(isBusy())
        ;

    //Select device, writing top 4 bits to 0x1f6 in the process
    outb(0x1f6, 0xe0 | sector >> 24);

    //Turn off interrupts
    outb(0x3f6, 0x2);

    //Write sector count to port 0x1f2
    outb(0x1f2, 0x1);

    //Write sector number (If you ask for multiple sectors, you must read all the data from the device before issuing another request)
    //Low 8 bits to 0x1f3
    outb(0x1f3, 0xff & sector);

    //Next 8 bits to 0x1f4
    outb(0x1f4, sector >> 8);

    //Next 8 bits to 0x1f5
    outb(0x1f5, sector >> 16);

    //poll the “drive ready” bit between each sector transferred
    while(isBusy())
        ;

/*==========================================================================*/

    //DISK READ CODE

    //initiate the read
    outb(0x1f7, 0x20);

    //wait until disk controller is no longer busy
    while(isBusy())
        ;

    //wait for the disk to be ready
    /*
        Read from port 0x1f7
            If bit 3 (=8) set: Data ready
            If bit 0 (=1) or bit 5 (=32) set: Error
        Keep looping until one of bits (0,3,5) set
    */

    unsigned char c;
    while(!((c=inb(0x1f7))&8))
        ;

    // Read!
    int i;
    unsigned short * db = (unsigned short *) datablock;
    for(i=0; i<256; ++i)
    {
        unsigned short d = inw(0x1f0); //...store d to memory...
        db[i] = d;
        db[i+1] = d >> 8;

        /*If reading more than one sector, adjust loop count accordingly
        Also need to wait for drive ready bit between each sector*/
    }

    return (int)((inb(0x1f7))&8);

}

void disk_write_sector(unsigned sector, const void* datablock)
{
    // Broken - change all outw to outb (all of them?)
    //SETUP CODE
    while(isBusy())
        ;
    //Select device
    outb(0x1f6, 0xe0 | sector >> 24);

    //Turn off interrupts
    outb(0x3f6, 0x2 | sector >> 24);

    //Write sector count to port 0x1f2
    outb(0x1f2, 0x1);

    //Write sector number (If you ask for multiple sectors, you must read all the data from the device before issuing another request)
    //Low 8 bits to 0x1f3
    outb(0x1f3, 0xff & sector);

    //Next 8 bits to 0x1f4
    outb(0x1f4, sector >> 8);

    //Next 8 bits to 0x1f5
    outb(0x1f5, sector >> 16);

    //poll the “drive ready” bit between each sector transferred
    while(isBusy())
        ;

/*==========================================================================*/

    //DISK WRITE CODE

    //initiate the write
    outb(0x1f7, 0x30);

    //wait until disk controller is no longer busy
    while(isBusy())
        ;

    /*wait for the disk to be ready
    Read from port 0x1f7
        If bit 3 (=8) set: Data ready
        If bit 0 (=1) or bit 5 (=32) set: Error
    Keep looping until one of bits (0,3,5) set
    */
    unsigned char c;
    while(!((c=inb(0x1f7))&8))
        ;

    // Write!
    int i;
    short* p = (short *) datablock;   //...data to write...
    for(i=0; i<256; ++i)
    {
        outb( 0x1f0, *p );
        p++;
    }

    //flush cache when we're finished with writes
    outb(0x1f7, 0xe7);
}
