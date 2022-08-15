#include "syscall.h"
#include "lib.c"

int
main(int argc, char *argv[])
{
    if (argc < 2) {
        puts2("Missing Arguments\n");
        Exit(1);
    }

    char c[1];
    for (int i = 1; i < argc; ++i) {
        OpenFileId file = Open(argv[i]);
        if (file < 0) {
            puts2("Error Opening File\n");
            Exit(1);
        }

        while (Read(c, 1, file)) {
            Write(c, 1, CONSOLE_OUTPUT);
        }
        Write("\n", 1, CONSOLE_OUTPUT);
        Close(file);
    }
    
    return 0;
}
