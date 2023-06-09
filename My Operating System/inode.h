#pragma pack(push,1)
struct Inode {
    unsigned short mode;
    unsigned short uid;
    unsigned size;
    unsigned atime;
    unsigned ctime;
    unsigned mtime;
    unsigned dtime;
    unsigned short gid;
    unsigned short links;
    unsigned blocks;
    unsigned flags;
    unsigned osd1;
    unsigned direct[12];
    unsigned indirect;
    unsigned doubleindirect;
    unsigned tripleindirect;
    unsigned generation;
    unsigned fileacl;
    unsigned diracl;
    unsigned osd2;
    char reserved[12];
};
#pragma pack(pop)
