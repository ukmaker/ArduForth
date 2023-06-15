#ifndef UKMAKER_MEMORY_H
#define UKMAKER_MEMORY_H
#include "Arduino.h"
/**
 * Abstractts two blocks of memory - ROM and RAM
 * In an MCU ROM will be implemented in Flash
*/

class Memory {

    public:

    Memory(uint8_t *ram, size_t ramSize, uint16_t ramStart,
        const uint8_t *rom, size_t romSize, uint16_t romStart) :
        _ram(ram), _ramSize(ramSize), _ramStart(ramStart),
        _rom(rom), _romSize(romSize), _romStart(romStart)
         {
            _ramEnd = _ramStart + _ramSize;
            _romEnd = _romStart + _romSize;
         }

    ~Memory() {}

    virtual void put(uint16_t addr, uint16_t w)=0;

    virtual uint16_t get(uint16_t addr)=0;

    virtual void putC(uint16_t addr, uint8_t c)=0;

    virtual uint8_t getC(uint16_t addr)=0;

    virtual uint8_t *addressOfChar(uint16_t location)=0;

    virtual uint16_t *addressOfWord(uint16_t location)=0;

    protected:

    uint8_t *_ram;
    uint16_t _ramSize;
    uint16_t _ramStart;
    uint16_t _ramEnd;

    const uint8_t *_rom;
    uint16_t _romSize;
    uint16_t _romStart;
    uint16_t _romEnd;

};
#endif