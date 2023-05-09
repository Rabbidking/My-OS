#pragma once
#include "superblock.h"
#include "inode.h"
#include "errno.h"

void init_filesystem(struct Superblock* sb);
void disk_read_block(unsigned blocknum, void* p);
void disk_read_multi_sector(unsigned start_sector, unsigned num_sector, void* p);
void get_BGD(int blockgroup, void* p);
void print_block();
int disk_read_block_partial(unsigned block, void* vp, unsigned start, unsigned count);
void disk_read_inode(unsigned num, struct Inode* ino);
void list_directory(int dir_inode, int indent);
unsigned get_file_inode(unsigned dir_inode, char* file_name);
int file_open(const char* fname, int flags);
int file_close(int fd);
