#ifndef UKMAKER_SYSCALLS_H
#define UKMAKER_SYSCALLS_H

#include "ForthVM.h"
#include "Serial.h"
#include <string.h>

#define SYSCALL_PRINTC 0
#define SYSCALL_TYPE 1
#define SYSCALL_DOT 2
#define SYSCALL_GETC 3
#define SYSCALL_PUTC 4
#define SYSCALL_INLINE 5
#define SYSCALL_FLUSH 6
#define SYSCALL_NUMBER 7

void syscall_printC(ForthVM *vm) {
    // Syscall to print the char on the top of the stack
    // ( c - )
    uint16_t v = vm->pop();
    char c = (char) v;

    printf("%c", c);
}

void syscall_type(ForthVM *vm) {
    // Syscall to print the string pointed to by the top of stack
    // The forst word of the string is the length
    // ( addr - )
    uint16_t dp = vm->pop();
    uint16_t len = vm->read(dp); // length in bytes
    dp+=2;
    uint8_t b;
    for(uint16_t i=0; i<len; i++) {
        b = vm->readByte(dp+i);
        printf("%c", b);    
    }
    fflush(stdout);
}

void syscall_dot(ForthVM *vm) {
    // Syscall to print a value on the stack
    // 
    // ( v base - )
    int base = vm->pop();
    int v = vm->pop();
    switch(base) {
        case 1:    
            printf("%04x", v);
            break;
        case 2:
            for(uint8_t i=0; i<16; i++) {
                if (v & 1)
                    printf("1");
                else
                    printf("0");

                v >>= 1;
            }
            printf("\n");
            break;
        case 0: 
        default:
            printf("%4d", v);
            break;
        
    }
}

void syscall_getc(ForthVM *vm) {
    int c = Serial::getc();
    vm->push(c);
}

void syscall_putc(ForthVM *vm) {
    int i = Serial::putc(vm->pop());
    vm->push(i);
}

void syscall_inline(ForthVM *vm) {
    // address of the buffer struct
    uint16_t buf = vm->pop();
    uint8_t *cbuf = vm->ram()->addressOfChar(buf+4);
    if(fgets((char *)cbuf, 127, stdin) != NULL) {
        // There will now be a null-terminated string in the buffer
        // calculate the end and store that in buf+2
        vm->ram()->put(buf+2, buf + 4 + strlen((char *)cbuf));
        // Current buffer pointer is just the start of the buffer
        vm->ram()->put(buf,buf+4);
        vm->push(0x01);
    } else {
        vm->push(0x00);
    }
}

void syscall_flush(ForthVM *vm) {
    fflush(stdout);
}

void syscall_number(ForthVM *vm) {
    uint16_t dp = vm->pop();
    uint16_t len = vm->ram()->get(dp);
    char *cbuf = (char *)vm->ram()->addressOfChar(dp+2);
    // put a null byte at the end
    vm->ram()->putByte(dp + len + 2, 0);
    int i;
    // We only deal with ints for now
    if(sscanf(cbuf, "%d", &i) == 1) {
        vm->push(i);
        vm->push(1);
    } else {
        vm->push(0);
    }
}

#endif