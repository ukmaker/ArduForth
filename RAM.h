#ifndef UKMAKER_RAM_H
#define UKMAKER_RAM_H

#include "Arduino.h"

class RAM {

    public:

    RAM(uint8_t *memory, size_t size) :
     _mem(memory), _size(size)
     {
        for(size_t i=0; i<size; i++) *memory++ = 0;
     }

     uint8_t getByte(uint16_t addr) {
        _check(addr);
         return *(_mem + addr);
     }

     void putByte(uint16_t addr, uint8_t b) {
        _checkWrite(addr);
         *(_mem + addr) = b;
     }

     uint16_t get(uint16_t addr) {
         _checkW(addr);
         return (uint16_t)*(_mem+addr) + ((uint16_t)*(_mem+addr+1) << 8);
     }

     void put(uint16_t addr, uint16_t data) {
         _checkWWrite(addr);
         _mem[addr] = (uint8_t)(data & 0xff);
         _mem[addr+1] = (uint8_t)(data >> 8);
     }

     uint8_t *addressOfChar(uint16_t location) {
         _check(location);
         return _mem + location;
     }

     uint16_t *addressOfWord(uint16_t location) {
         _checkW(location);
         return (uint16_t *)(_mem+location);
     }

     void setWatch(uint16_t addr) {
        _watch = true;
        _watched = addr;
     }

     void clearWatch() {
        _watch = false;
     }

     void writeProtect(uint16_t start, uint16_t end) {
        _protectedStart = start;
        _protectedEnd = end;
     }


     protected:

     uint8_t *_mem;
     size_t _size;
     bool _watch = false;
     uint16_t _watched = 0;
     uint16_t _protectedStart = 0;
     uint16_t _protectedEnd = 0;

     void _check(uint16_t addr) {
        if((addr) > _size) {
            //printf("WARNING: access out of bounds - %d > %d\n", addr, _size);
        }
        if(_watch && addr == _watched) {
            printf("ACCESS TO WATCH\n");
        }
     }
     void _checkW(uint16_t addr) {
        _check(addr);
        if((addr & 1) != 0) {
            printf("WARNING: unaligned access - %d\n", addr);
        }
     }

     void _checkWrite(uint16_t addr) {
        _check(addr);
        if(addr >= _protectedStart && addr < _protectedEnd) {
            printf("WARNING: write to protected region %04x\n", addr);
        }        
     }

     void _checkWWrite(uint16_t addr) {
        _checkW(addr);

        if(addr >= _protectedStart && addr < _protectedEnd) {
            printf("WARNING: write to protected region %04x\n", addr);
        }
     }

};
#endif