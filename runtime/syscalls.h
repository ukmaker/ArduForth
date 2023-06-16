#ifndef UKMAKER_SYSCALLS_H
#define UKMAKER_SYSCALLS_H

#include <Arduino.h>
#include "ForthVM.h"
#include "Serial.h"

#define SYSCALL_DEBUG 0
#define SYSCALL_TYPE 1
#define SYSCALL_TYPELN 2
#define SYSCALL_DOT 3
#define SYSCALL_GETC 4
#define SYSCALL_PUTC 5
#define SYSCALL_INLINE 6
#define SYSCALL_FLUSH 7
#define SYSCALL_NUMBER 8
#define SYSCALL_D_AT 9
#define SYSCALL_D_STORE 10
#define SYSCALL_D_ADD 11
#define SYSCALL_D_SUB 12
#define SYSCALL_D_MUL 13
#define SYSCALL_D_DIV 14

void syscall_type(ForthVM *vm)
{
    // Syscall to print the string pointed to by the top of stack
    // The forst word of the string is the length
    // ( addr - )
    uint16_t dp = vm->pop();
    uint16_t len = vm->read(dp); // length in bytes
    dp += 2;
    uint8_t b;
    for (uint16_t i = 0; i < len; i++)
    {
        b = vm->readByte(dp + i);
        SerialUSB.printf("%c", b);
    }
    SerialUSB.flush();
}

void syscall_typeln(ForthVM *vm)
{
    syscall_type(vm);
    SerialUSB.print('\n');
    SerialUSB.flush();
}

void syscall_dot(ForthVM *vm)
{
    // Syscall to print a value on the stack
    //
    // ( v base - )
    int base = vm->pop();
    int v = vm->pop();
    switch (base)
    {
    case 16:
        SerialUSB.printf("0x%04x", v);
        break;
    case 2:
    {
        printf("0b");
        uint16_t mask = 0x8000;
        while (mask != 0)
        {
            if (v & mask)
                SerialUSB.print('1');
            else
                SerialUSB.print('0');

            mask >>= 1;
        }
        SerialUSB.print('\n');
    }
    break;
    case 10:
    default:
        SerialUSB.printf("%d", v);
        break;
    }
}

void syscall_getc(ForthVM *vm)
{
    int c = SerialUSB.read();
    vm->push(c);
}

void syscall_putc(ForthVM *vm)
{
    SerialUSB.print((char)vm->pop());
}

void syscall_inline(ForthVM *vm)
{
    // address of the buffer struct
    uint16_t buf = vm->pop();
    uint16_t bufidx = buf;
    uint16_t bufend = buf + 2;
    uint16_t bufstart = buf + 4;

    uint8_t *cbuf = vm->ram()->addressOfChar(bufstart);
    size_t read;
    while(!SerialUSB.available()) {}
    if ((read = SerialUSB.readBytesUntil(0x0a, (char *)cbuf, 127)) != 0)
    {
        // There will now be a null-terminated string in the buffer
        // calculate the end and store that in buf+2
        vm->ram()->put(bufend, bufstart + read - 1); //strlen((char *)cbuf));
        // Current buffer pointer is just the start of the buffer
        vm->ram()->put(bufidx, bufstart);
        vm->push(0x01);
    }
    else
    {
        vm->push(0x00);
    }
}

void syscall_flush(ForthVM *vm)
{
    SerialUSB.flush();
}

void syscall_number(ForthVM *vm)
{
    uint16_t base = vm->pop();
    uint16_t dp = vm->pop();
    uint16_t len = vm->ram()->get(dp);
    char *cbuf = (char *)vm->ram()->addressOfChar(dp + 2);
    // put a null byte at the end
    vm->ram()->putC(dp + len + 2, 0);
    int i, r;
    i = 0;
    char c;

    switch (base)
    {
    case 16:
        r = sscanf(cbuf, "%x", &i);
        break;
    case 2:
        r = 0;
        while ((c = *cbuf++) != '\0') {
            i <<= 1;
            if(c == '1') {
                i += 1;
                r = 1;
            } else if(c == '0') {
                r = 1;
            } else {
                break;
            }
        }
        
        break;
    case 10:
    default:
        r = sscanf(cbuf, "%d", &i);
        break;
    }

    if (r == 1)
    {
        vm->push(i);
        vm->push(1);
    }
    else
    {
        vm->push(0);
    }
}

// to interface with the underlying hardware
// these syscalls are needed to do 32-bit reads and writes on an STM32
void syscall_write_double(ForthVM *vm) {
    uint16_t h = vm->pop();
    uint16_t l = vm->pop();
    uint32_t addr = l + (h << 16);
    h = vm->pop();
    l = vm->pop();
    uint32_t data = l + (h << 16);
    *(uint32_t *)addr = data;
}

void syscall_read_double(ForthVM *vm) {
    uint16_t h = vm->pop();
    uint16_t l = vm->pop();
    uint32_t addr = l + (h << 16);
    uint32_t data = *(uint32_t *)addr;
    vm->push(data & 0x0000ffff);
    vm->push(data >> 16);
}

void syscall_add_double(ForthVM *vm) {
    uint16_t h = vm->pop();
    uint16_t l = vm->pop();
    uint32_t arga = l + (h << 16);   
    h = vm->pop();
    l = vm->pop();
    uint32_t argb = l + (h << 16); 
    uint32_t result = arga + argb;
    vm->push(result & 0x0000ffff);
    vm->push(result >> 16);
}

void syscall_sub_double(ForthVM *vm) {
    uint16_t h = vm->pop();
    uint16_t l = vm->pop();
    uint32_t arga = l + (h << 16);   
    h = vm->pop();
    l = vm->pop();
    uint32_t argb = l + (h << 16); 
    uint32_t result = argb - arga;
    vm->push(result & 0x0000ffff);
    vm->push(result >> 16);
}

void syscall_mul_double(ForthVM *vm) {
    uint16_t h = vm->pop();
    uint16_t l = vm->pop();
    uint32_t arga = l + (h << 16);   
    h = vm->pop();
    l = vm->pop();
    uint32_t argb = l + (h << 16); 
    uint32_t result = arga * argb;
    vm->push(result & 0x0000ffff);
    vm->push(result >> 16);
}

void syscall_div_double(ForthVM *vm) {
    uint16_t h = vm->pop();
    uint16_t l = vm->pop();
    uint32_t arga = l + (h << 16);   
    h = vm->pop();
    l = vm->pop();
    uint32_t argb = l + (h << 16); 
    uint32_t result = argb / arga;
    vm->push(result & 0x0000ffff);
    vm->push(result >> 16);
}


#endif