#include "syscall.h"
#include "lib.c"

int
main(int argc, char *argv[])
{
    if (argc < 3) {
        puts2("Missing Arguments\n");
        Exit(1);
    }

    OpenFileId sourceFile = Open(argv[1]);
    if (sourceFile < 0) {
        puts2("Error Opening Source File\n");
        Exit(1);
    }

    Remove(argv[2]);
    Create(argv[2]);
    OpenFileId destinationFile = Open(argv[2]);
    if (sourceFile < 0) {
        puts2("Error Opening Destination File\n");
        Exit(1);
    }

    char c[1];
    while(Read(c, 1, sourceFile)) {
        Write(c, 1, destinationFile);
    }

    Close(sourceFile);
    Close(destinationFile);
    
    return 0;
}
