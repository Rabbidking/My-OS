#include "Multiboot.h"
#include "console.h"
#include "utils.h"
#include "file.h"
//#include "interrupts.h"
char buffer[4096];

void sweet();

void clearBss(char* bssStart, char* bssEnd) {
	//write zeros to memory (MUST be a loop!)
	while (bssStart != bssEnd)
	{
		*bssStart = 0;
		bssStart++;
	}
}

void kmain(struct MultibootInfo * mbi){
    clearBss(0, (char*) 1024);
    console_init(mbi);

    disk_read_multi_sector(2, 4, buffer);
    struct Superblock* sb = (struct Superblock*)buffer;
    init_filesystem(sb);
    //print_block();
    //list_directory(2, 0);
    sweet();

    //interrupt_init();
    //exec("usertest1.bin");

    while(1){
    	asm volatile("hlt");
    }
}
