#ifndef UKMAKER_UNSAFE_MEMORY_H
#define UKMAKER_UNSAFE_MEMORY_H

#include "Memory.h"

class UnsafeMemory : public Memory {

public:

    UnsafeMemory(uint8_t *ram, size_t ramSize, uint16_t ramStart,
        const uint8_t *rom, size_t romSize, uint16_t romStart) : 
        Memory(ram, ramSize, ramStart, rom, romSize, romStart) {}

    ~UnsafeMemory() {}

    void put(uint16_t addr, uint16_t w) {
        if(addr >= _ramStart && addr <= _ramEnd) {
            *(uint16_t *)(_ram + addr - _ramStart) = w;
        } else {
            *(uint16_t *)(_rom + addr - _romStart) = w;
        }
    }

    uint16_t get(uint16_t addr) {

        if(addr >= _ramStart && addr <= _ramEnd) {
            return *(uint16_t *)(_ram + addr - _ramStart);
        } else {
            return *(uint16_t *)(_rom + addr - _romStart);
        }
    }

    void putC(uint16_t addr, uint8_t c) {
         if(addr >= _ramStart && addr <= (_ramEnd+1)) {
            *(_ram + addr - _ramStart) = c;
         } else {
            *(uint8_t *)(_rom + addr - _romStart) = c;
         }
    }

    uint8_t getC(uint16_t addr) {
        
        if(addr >= _ramStart && addr <= (_ramEnd+1)) {
            return *(_ram + addr - _ramStart);
        } else {
            return *(_rom + addr - _romStart);
        }
    }

    uint8_t *addressOfChar(uint16_t addr) {
         if(addr >= _ramStart && addr <= (_ramEnd+1)) {
            return _ram + addr - _ramStart;
         } else {
            return (uint8_t *)(_rom + addr - _ramStart);
         }
     }

     uint16_t *addressOfWord(uint16_t addr) {
        if(addr >= _ramStart && addr <= _ramEnd) {
            return (uint16_t *)(_ram + addr - _ramStart);
        } else {
            return (uint16_t *)(_rom + addr - _romStart);
         }
     }
};
#endif