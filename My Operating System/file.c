#include "superblock.h"
#include "inode.h"
#include "BlockGroupDescriptor.h"
#include "disk.h"
#include "utils.h"
#include "DirEntry.h"
#include "console.h"
#include "errno.h"

#define MAX_FILES 24

volatile struct Superblock* sb;
char buffer1[8192];
char buffer2[8192];
struct DirEntry* d;
struct Inode* ino;

struct File{
	int in_use;
	struct Inode ino;
};

union U{
    char block[4096];
    struct BlockGroupDescriptor bgd[1];    //variable size, not really 1
};

struct File* file_table[MAX_FILES];

void init_filesystem(struct Superblock* s)
{
    //Superblock initialization
    sb = s;
    /*for(int i = 0; i < MAX_FILES; i++)
	{
        file_table[i]->in_use = 0;
	}*/
}

void disk_read_multi_sector(unsigned start_sector, unsigned num_sector, void* p)
{
    //read multiple disk sectors at once. Increment the offset by 512 for each sector
    int i;
    int offset = 0;
    for(i = start_sector; i < start_sector + num_sector; i++)
    {
        //disk_read_sector(i, p + offset);
        disk_read_sector(i, p + offset, inb(0x1f7)&8);
        offset += 512;
    }
}

void disk_read_block(unsigned blocknum, void* p)
{
    disk_read_multi_sector(blocknum * 8, 8, p);
}

void get_BGD(int blockgroup, void* p)
{
    // Gets the Block Group Descriptor Table, using the read multi sector function
    disk_read_multi_sector((blockgroup * sb->blocks_per_group + 1) * 8, 8, p);
}

void print_block()
{
    // Prints information from the current block group descriptor table
    int i, j;
    static union U tmp;

    kprintf("Volume label: %s Free: %d\n", sb->volname, sb->free_block_count);
    kprintf("Blocks per group: %d Total blocks: %d\n", sb->blocks_per_group, sb->block_count);

    int blockGroupCount = sb->block_count / sb->blocks_per_group;
    for(i = 0; i <= blockGroupCount; i++)
    {
        kprintf("Reading BGDT from group %d\n", i);
        get_BGD(i, tmp.block);
        for(j = 0; j <= blockGroupCount; j++)
        {
            kprintf("Group %d: Free blocks = %d\n", j, tmp.bgd[j].free_blocks);
        }
    }
}


void disk_read_inode(unsigned inode_num, struct Inode* ino)
{
    //Finding the root directory
    int group = (inode_num - 1) / sb->inodes_per_group; //find block group number, using the passed-in inode number
    disk_read_block(group * sb->blocks_per_group + 4, buffer2);
    ino = (struct Inode*) buffer2;  //start of buffer1 into ino
    //And now we can fetch inode 2: ino[1]
    int inode_index = (inode_num - 1) % sb->inodes_per_group;    //fetches inode index, so we can go to specific inode index
    kmemcopy(buffer1, ino + inode_index, sizeof(struct Inode));
    ino = (struct Inode*) buffer1;
}

void disk_read_inode_table(unsigned inode_num, struct Inode* ino)
{
    int group = (inode_num - 1) / sb->inodes_per_group; //find block group number, using the passed-in inode number
    disk_read_block(group * sb->blocks_per_group + 4, ino);
}

void list_directory(int dir_inode, int indent)
{
    //start indent at 0, each new call, increment by 1
    //instead of pointer arthmatic, increment pointer by size rec_length

    ino = (struct Inode*) buffer1;
    char* p = buffer2;  //start of buffer
    d = (struct DirEntry*) p;   //Set d to p (three pointers). Later, we'll need to do pointer arithmatic on p, NOT d
    disk_read_inode_table(dir_inode, ino);    //reads dir_inode into the ino
    //disk_read_block(ino->direct, ino);
    disk_read_block(ino[dir_inode - 1].direct[0], buffer2);

    //In buffer1, inode; in buffer2, have a block of directory entries
    //kprintf("%d", ino[dir_inode - 1].direct[0]);
    while(1)
    {
        if(d->rec_len == 0)
        {
            break;
        }

        //Write indent
        for(int i = 0; i < indent; i++)
        {
            console_putc('_');
        }

        //Write inode
        if(d->inode < 10)
        {
            kprintf("< %d>", d->inode);
        }
        else
        {
            kprintf("<%d>", d->inode);
        }
        char* file_name = d->name;
        console_putc(' ');
        for(int i = 0; i < d->name_len; i++)
        {
            console_putc(*file_name);
            file_name++;
        }
        console_putc('\n');
        //kprintf("%s\n", file_name);

        //subdirectory
        /*if(((ino[d->inode - 1].mode >> 12) & 0xff) == 4)  //gets mode for directory check
        {
            if(d->name[0] != '.')   //. = itself, .. = one level up (.. is the same as . in the root directory)
            {
                list_directory(d->inode, indent + 4);
                //reset the buffers to their initial states before this call
                disk_read_inode_table(dir_inode, ino);
                disk_read_block(ino[dir_inode - 1].direct[0], buffer2);
            }
        }*/
        p += d->rec_len;    //increment p by d's rec_length
        d = (struct DirEntry*) p;   //set d back equal to p
    }
}

unsigned get_file_inode(unsigned dir_inode, char* file_name)
{
    //returns inode of the given file name, if it exists. Otherwise, return null
    //almost like print_directory; instead of printing, compare directory to name passed in

    ino = (struct Inode*) buffer1;
    char* p = buffer2;
    d = (struct DirEntry*) p;
    disk_read_inode_table(dir_inode, ino);
    disk_read_block(ino[dir_inode - 1].direct[0], buffer2);
    int cur_rec_length = 0;

    while(cur_rec_length < 4096)
    {
        if(d->rec_len == 0)
        {
            break;
        }

        kprintf("%*s\n", d->name_len, d->name);
        //kprintf("%s\n", file_name);
        if(kmemcmp(d->name, file_name, d->name_len) == 0)
        {
            //kprintf("%s*****\n", file_name);
            return d->inode;
        }

        p += d->rec_len;
        cur_rec_length += d->rec_len;
        d = (struct DirEntry*) p;
    }
    return -ENOENT;
}

int file_open(char* fname, int flags){
	int fd;
	int freeSpace = 0;
	for(int i = 0; i < MAX_FILES; i++)
	{
		/*Checks free space in file_table
		if no space in file table:
			return -EMFILE;*/
        //kprintf("%d = %d\n", i, file_table[i]->in_use);
        if(file_table[i]->in_use == 0)
        {
            freeSpace = 1;
            fd = i; //file descriptor, the index of free entry in file_table
            break;
        }
	}
	kprintf("Freespace = %d\n", freeSpace);
	if(freeSpace == 0)
    {
        return -EMFILE;
    }

	file_table[fd]->in_use = 1;
	int fileCheck = get_file_inode(2, fname);
	if(fileCheck < 0)
    {
        file_table[fd]->in_use = 0;
        return -ENOENT; //no file found!
    }
    disk_read_inode(fileCheck, &file_table[fd]->ino);
    file_table[fd]->in_use = 0;
    return fd;
}

int file_close(int fd){
	//close the file
	if(fd < 0 || fd >= MAX_FILES || file_table[fd]->in_use == 0)
		return -EINVAL;
	file_table[fd]->in_use = 0;
	return 0;
}
