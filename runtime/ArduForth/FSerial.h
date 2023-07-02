#ifndef UKMAKER_FSERIAL_H
#define UKMAKER_FSERIAL_H
#include <stdio.h>
#ifndef ARDUINO


class FSerial {

    public:
    
    int getc() {
        return getchar();
    }

    int putc(char c) {
        return putchar(c);
    }
    
    void printf(const char *format, int value) {
        ::printf(format, value);
    }

    void flush() {
        // no op
    }

    void print(char c) {
        putchar(c);
    }

    int read() {
        return getchar();
    }

    bool available() {
        return true;
    }

    int readBytesUntil(char separator, char *buf, size_t len) {
        char c;
        size_t read = 1;
        while((c = getc()) != separator && read < len) {
            *buf++ = c;
            read++;
        }
        
        return read;
    }


};

FSerial Serial;

#endif
#endif