#include "syscall.h"
#include "lib.c"

int
main(int argc, char *argv[])
{
    if (argc < 2) {
        puts2("Missing Arguments\n");
        Exit(1);
    }

    for (int i = 1; i < argc; ++i) {
        if (Remove(argv[1]) == -1) {
            puts2("Error couldn't delete file\n");
        } else {
            puts2("File deleted\n");
        }
    }
    
    return 0;
}
