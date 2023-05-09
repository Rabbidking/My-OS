#pragma pack(push,1)
struct DirEntry{
    unsigned inode;
    unsigned short rec_len;
    unsigned short name_len;
    char name[1]; //might be longer! Variable size
};
#pragma pack(pop)
