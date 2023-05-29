#ifndef UKMAKER_RAM_H
#define UKMAKER_RAM_H

#include "Arduino.h"

class RAM {

    public:

    RAM(uint16_t *memory, size_t size) :
     _mem(memory), _size(size)
     {}

     uint8_t getByte(uint16_t addr) {
        return *((uint8_t *)_mem + addr);
     }

     void putByte(uint16_t addr, uint8_t b) {
        *((uint8_t *)_mem + addr) = b;
     }

     uint16_t get(uint16_t addr) {
      return _mem[addr>>1];
     }

     void put(uint16_t addr, uint16_t data) {
      _mem[addr>>1] = data;
     }


     protected:

     uint16_t *_mem;
     size_t _size;

};
#endif