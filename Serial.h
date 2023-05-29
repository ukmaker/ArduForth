#ifndef UKMAKER_SERIAL_H
#define UKMAKER_SERIAL_H
#include <stdio.h>
class Serial {

    public:
    
    static int getc() {
        return getchar();
    }

    static int putc(char c) {
        return putchar(c);
    }

};
#endif