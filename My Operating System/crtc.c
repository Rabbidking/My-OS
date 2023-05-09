extern unsigned bssStart,bssEnd;
extern int main();
void _start(){
    //clear the bss
    char* p = (char*)&bssStart;
    while(p != (char*)&bssEnd){
        *p=0;
        p++;
    }
    main();
}
