#pragma once
#pragma pack(push,1)

struct BlockGroupDescriptor{
    unsigned block_bitmap;  //id of the first block bitmap
    unsigned inode_bitmap;  //id of the first inode bitmap
    unsigned inode_table;   //id of the first inode table
    unsigned short free_blocks; //total number of free blocks
    unsigned short free_inodes; //total number of free inodes
    unsigned short used_dirs;   //number of inodes allocated to directories
    unsigned short pad;     //used for padding the structure on a 32bit boundary
    char reserved[12];      //reserved space for future revisions
};
#pragma pack(pop)
