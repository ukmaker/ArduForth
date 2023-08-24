#ifndef UKMAKER_LOCAL_SYSCALLS_H
#define UKMAKER_LOCAL_SYSCALLS_H

#include "FArduino.h"
#include "ForthVM.h"

#define SYSCALL_FOPEN 32
#define SYSCALL_FCLOSE 33
#define SYSCALL_FREAD 34

extern char *source_code;
char *source_ptr;

/**
 * Syscalls to be used in an embedded environment
 * Useful if there is no filesystem in the environment
 * Mocks the fopen/fclose and fread calls to allow
 * Forth code to be read from a string
*/

// ( name-address type -- success )
void syscall_fopen(ForthVM *vm) {
    uint16_t type = vm->pop(); // 0 == read ; 1 = write
    uint16_t fname = vm->pop(); // points to a Forth string
    if(type == 0 && strlen(source_code) > 0) {
        source_ptr = source_code;
        vm->push(1);
    } else {
        vm->push(0);
    }
}

void syscall_fclose(ForthVM *vm) {
    uint16_t type = vm->pop();
}

void syscall_fread(ForthVM *vm) {
    // ( bufferAddress -- bytesRead ) bytesRead is -1 if EOF is reached
    
    // address of the buffer struct
    uint16_t buf = vm->pop();
    uint16_t bufidx = buf;
    uint16_t bufend = buf + 2;
    uint16_t bufstart = buf + 4;
    uint8_t *cbuf = vm->ram()->addressOfChar(bufstart);

    if(*source_code == '\0') {
        vm->push(-1);
        return;
    }

    uint16_t i = 0;
    int c;
    while( i < 128) {
        c = *source_code++;
        if(c == '\0') {
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