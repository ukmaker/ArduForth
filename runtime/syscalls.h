#ifndef UKMAKER_SYSCALLS_H
#define UKMAKER_SYSCALLS_H

#include "Arduino.h"
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
#define SYSCALL_H_AT 9
#define SYSCALL_H_STORE 10
#define SYSCALL_D_ADD 11
#define SYSCALL_D_SUB 12
#define SYSCALL_D_MUL 13
#define SYSCALL_D_DIV 14
#define SYSCALL_D_SR 15
#define SYSCALL_D_SL 16
#define SYSCALL_D_AND 17
#define SYSCALL_D_OR 18
#define SYSCALL_D_INVERT 19
#define SYSCALL_DOTC 20

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

void syscall_dot_c(ForthVM *vm)
{
    // Syscall to print the low byte of a value on the stack
    //
    // ( v base - )
    int base = vm->pop();
    int v = (uint8_t) vm->pop();
    switch (base)
    {
    case 16:
        SerialUSB.printf("0x%02x", v);
        break;
    case 2:
    {
        printf("0b");
        uint16_t mask = 0x80;
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

void _parse_binary(ForthVM *vm, char *cbuf, uint16_t len) {
    uint16_t v = 0;
    bool valid = true;
    for(uint16_t i=0; i<len; i++) {
        char c = *cbuf;
        cbuf++;
        if(c == '0') {
            v = v << 1;
        } else if(c == '1') {
            v = (v << 1) + 1;
        } else {
            valid = false;
        }
        if(!valid) break;
    }
    if(valid) {
        vm->push(v);
    }
    vm->push(valid ? 1 : 0);
}

int cToI(char c) {
    // Attempt to convert c to a valid number
    // recognises 0-9, A-F, a-f
    if(c >= '0' && c <= '9') {
        return c - '0';
    }

    if(c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }

    if(c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }

    return -1;
}

void _parse_hex(ForthVM *vm, char *cbuf, uint16_t len) {
    uint16_t v = 0;
    bool valid = true;
    for(uint16_t i=0; i<len; i++) {
        char c = *cbuf;
        cbuf++;
        int h = cToI(c);
        if(h == -1) {
            valid = false;
            break;
        }
        v = (v << 4) + h;
    }
    if(valid) {
        vm->push(v);
    }
    vm->push(valid ? 1 : 0);    
}

void _parse_decimal(ForthVM *vm, char *cbuf, uint16_t len, bool negative) {
    uint16_t v = 0;
    bool valid = true;
    for(uint16_t i=0; i<len; i++) {
        char c = *cbuf;
        cbuf++;
        int h = cToI(c);
        if(h == -1 || h > 9) {
            valid = false;
            break;
        }
        v = (v * 10) + h;
    }
    if(negative) v = -v;
    if(valid) {
        vm->push(v);
    }
    vm->push(valid ? 1 : 0);    
}

void _parse_char(ForthVM *vm, char *cbuf, uint16_t len) {
    uint16_t v = 0;
    bool valid = true;
    if(len == 0 || len > 3) {
        vm->push(0);
        return;
    }
    if(*cbuf == '\\' && len == 3) {
        cbuf++;
        len--;
        switch(*cbuf) {
            case 'n': v = '\n'; break;
            case 't': v = '\t'; break;
            case 'r': v = '\r'; break;
            case '0': v = 0; break;
            default: vm->push(0); return;
        }
        cbuf++;
        if(*cbuf != '\'') {
            vm->push(0);
        } else {
            vm->push(v);
            vm->push(1);
        }
        return;
    }

    if(len == 3) {
        vm->push(0);
        return;
    }

    if(*(cbuf+1) != '\'') {
        vm->push(0);
        return;
    }

    vm->push(*cbuf);
    vm->push(1);
}
/**
 * Attempt to convert the current token into a number
 * This does not use random weird characters to indicate 
 * the base as does e.g. GForth
 * 1. The system BASE variable is ignored for parsing
 *    it is only used for output. This way ambiguity is
 *    avoided.
 * 2. Supported bases are 
 *    decimal - no prefix
 *    hex     - prefix 0x
 *    binary  - prefix 0b
 * 3. A leading '-' indicates a negative number 
 *    for decimal numbers only
 *    is an error for other bases
 * 4. a character literal may be represented
 *    'c'      - the trailing ' is not optional as in GForth
 *    '\n'     - backslashes are allowed for \n, \t, \r and \0
 * 
*/
void syscall_number(ForthVM *vm)
{
    uint16_t base = vm->pop();
    uint16_t dp = vm->pop();
    uint16_t len = vm->ram()->get(dp);
    char *cbuf = (char *)vm->ram()->addressOfChar(dp + 2);
    // is there even something to parse?
    if(len == 0) {
        vm->push(0);
        return;
    }

    if(len == 1) {
        return _parse_decimal(vm, cbuf, len, false);
    }

    if(*cbuf == '-') {
        cbuf++;
        len--;
        return _parse_decimal(vm, cbuf, len, true);
    }

    if(*cbuf == '\'') {
        cbuf++;
        len--;
        return _parse_char(vm, cbuf, len);
    }

    if(len >= 3 && *cbuf == '0') {
        // check for leading base specifier
        if(*(cbuf+1) == 'b') {
            cbuf+=2;
            len -=2;
            return _parse_binary(vm, cbuf, len);
        }

        if(*(cbuf+1) == 'x') {
            cbuf+=2;
            len -=2;
            return _parse_hex(vm, cbuf, len);
        }
    }

    _parse_decimal(vm, cbuf, len, false);
}

// to interface with the underlying hardware
// these syscalls are needed to do 32-bit reads and writes on an STM32
void syscall_write_host(ForthVM *vm) {
    uint16_t h = vm->pop();
    uint16_t l = vm->pop();
    uint32_t addr = l + (h << 16);
    h = vm->pop();
    l = vm->pop();
    uint32_t data = l + (h << 16);
    *(uint32_t *)addr = data;
}

void syscall_read_host(ForthVM *vm) {
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

// ( Dvalue Sshift -- Dvalue )
void syscall_sr_double(ForthVM *vm) { 
    uint16_t shift = vm->pop();
    uint16_t h = vm->pop();
    uint16_t l = vm->pop();
    uint32_t val = l + (h << 16);   

    uint32_t result = val >> shift;
    vm->push(result & 0x0000ffff);
    vm->push(result >> 16);
}

void syscall_sl_double(ForthVM *vm) {
    uint16_t shift = vm->pop();
    uint16_t h = vm->pop();
    uint16_t l = vm->pop();
    uint32_t val = l + (h << 16);   

    uint32_t result = val << shift;
    vm->push(result & 0x0000ffff);
    vm->push(result >> 16);
}

void syscall_and_double(ForthVM *vm) {
    uint16_t h = vm->pop();
    uint16_t l = vm->pop();
    uint32_t arga = l + (h << 16);   
    h = vm->pop();
    l = vm->pop();
    uint32_t argb = l + (h << 16); 
    uint32_t result = argb & arga;
    vm->push(result & 0x0000ffff);
    vm->push(result >> 16);
}

void syscall_or_double(ForthVM *vm) {
    uint16_t h = vm->pop();
    uint16_t l = vm->pop();
    uint32_t arga = l + (h << 16);   
    h = vm->pop();
    l = vm->pop();
    uint32_t argb = l + (h << 16); 
    uint32_t result = argb | arga;
    vm->push(result & 0x0000ffff);
    vm->push(result >> 16);
}

void syscall_invert_double(ForthVM *vm) {
    uint16_t h = vm->pop();
    uint16_t l = vm->pop();
    uint32_t arga = l + (h << 16);   
    uint32_t result = ~arga;
    vm->push(result & 0x0000ffff);
    vm->push(result >> 16);
}


#endif