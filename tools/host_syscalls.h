#ifndef UKMAKER_HOST_SYSCALLS_H
#define UKMAKER_HOST_SYSCALLS_H

#include "../runtime/Arduino.h"
#include "../runtime/ForthVM.h"
#include <stdio.h>

#define SYSCALL_FOPEN 21
#define SYSCALL_FCLOSE 22
#define SYSCALL_FREAD 23

/**
 * Syscalls to be used during development
 * E.g. to allow loading of Forth source files for
 * compilation into a runtime image
*/

static FILE *reader;
static FILE *writer;
// ( name-address type -- success )
void syscall_fopen(ForthVM *vm) {
    uint16_t type = vm->pop(); // 0 == read ; 1 = write
    uint16_t fname = vm->pop(); // points to a Forth string
    char *name = (char *)vm->ram()->addressOfChar(fname+2);
    uint16_t len = vm->read(fname);
    len = len & 0x3fff; // Mask off the immediate/exec bits
    char *cname;
    cname = (char *)malloc(len+1);
    for(uint16_t i=0; i<len; i++ ) {
        cname[i] = vm->readByte(fname + 2 + i);
    }
    cname[len] = '\0';

    const char *r = "r";
    const char *w = "w";
    bool success = true;

    if(type == 0) {
        reader = fopen(cname, r);
        if(reader == NULL) success = false;
    } else {
        writer = fopen(cname, w);
        if(writer == NULL) success = false;
    }

    vm->push(success ? 1 : 0);
    free(cname);
}

void syscall_fclose(ForthVM *vm) {
    uint16_t type = vm->pop();
    if(type == 0) {
        if(reader != NULL) fclose(reader);
        reader = NULL;
    } else {
        if(writer != NULL) fclose(writer);
        writer = NULL;
    }
}

void syscall_fread(ForthVM *vm) {
    // ( bufferAddress -- bytesRead ) bytesRead is -1 if EOF is reached
    
    // address of the buffer struct
    uint16_t buf = vm->pop();
    uint16_t bufidx = buf;
    uint16_t bufend = buf + 2;
    uint16_t bufstart = buf + 4;
    uint8_t *cbuf = vm->ram()->addressOfChar(bufstart);

    if(reader == NULL) {
        vm->push(-1);
        return;
    }

    uint16_t i = 0;
    int c;
    while( i < 128) {
        c = fgetc(reader);
        if(feof(reader)) {
            if(i == 0) {
                vm->push(-1);
            } else {
                vm->push(i);
            }
            return;
        }

        *cbuf++ = c;
        i++;
        if(c == 0x0a) {
            break;
        }
    }

    vm->ram()->put(bufend, bufstart + i - 1);
    // Current buffer pointer is just the start of the buffer
    vm->ram()->put(bufidx, bufstart);
    vm->push(i);
}



#endif