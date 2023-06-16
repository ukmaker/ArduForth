#ifndef UKMAKER_SAFE_MEMORY_H
#define UKMAKER_SAFE_MEMORY_H


#include "Memory.h"

#define MEMORY_ACCESS_RAM 0
#define MEMORY_ACCESS_ROM 1
#define MEMORY_ACCESS_INVALID 2

using InvalidAccess = void (*)(uint16_t);

class SafeMemory : public Memory {

    public:

    SafeMemory(uint8_t *ram, size_t ramSize, uint16_t ramStart,
        const uint8_t *rom, size_t romSize, uint16_t romStart) :
        Memory(ram, ramSize, ramStart, rom, romSize, romStart) {}

    ~SafeMemory() {}


    void put(uint16_t addr, uint16_t w) {
        if(_checkWWrite(addr)) {
            *(uint16_t *)(_ram + addr - _ramStart) = w;
        }
    }

    uint16_t get(uint16_t addr) {
        
        uint16_t rv = 0;

        switch(_checkWRead(addr)) {
            case MEMORY_ACCESS_RAM:
                rv = *(uint16_t *)(_ram + addr - _ramStart);
                break;

            case MEMORY_ACCESS_ROM:
                rv = *(uint16_t *)(_rom + addr - _romStart);
                break;

            default: 
                break;
        }
        return rv;
    }

    void putC(uint16_t addr, uint8_t c) {
        if(_checkCWrite(addr)) {
            *(_ram + addr - _ramStart) = c;
        }
    }

    uint8_t getC(uint16_t addr) {
        
        uint8_t rv = 0;

        switch(_checkCRead(addr)) {
            case MEMORY_ACCESS_RAM:
                rv = *(_ram + addr - _ramStart);
                break;

            case MEMORY_ACCESS_ROM:
                rv = *(_rom + addr - _romStart);
                break;

            default: 
                break;
        }
        return rv;
    }

    uint8_t *addressOfChar(uint16_t location) {
         switch(_checkCRead(location)) {
            case MEMORY_ACCESS_RAM: return _ram + location - _ramStart;
            //case MEMORY_ACCESS_ROM: return (uint8_t *)(_rom + location - _ramStart);
            default: break;

         }
         return NULL;
     }

     uint16_t *addressOfWord(uint16_t location) {
         switch(_checkWRead(location)) {
            case MEMORY_ACCESS_RAM: return (uint16_t *)(_ram + location - _ramStart);
            case MEMORY_ACCESS_ROM: return (uint16_t *)(_rom + location - _romStart);
            default: break;
         }
         return NULL;
     }

    void attachUnalignedWriteCallback(InvalidAccess fp);
    void attachROMWriteCallback(InvalidAccess fp);
    void attachUndefinedAccessCallback(InvalidAccess fp);

    protected:

    InvalidAccess _unalignedWrite;
    InvalidAccess _romWrite;
    InvalidAccess _undefinedAccess;

    bool _checkWWrite(uint16_t addr) {
        return _checkWrite(addr);
    }

    uint8_t _checkWRead(uint16_t addr) {
        return _checkRead(addr);
    }

    bool _checkCWrite(int16_t addr) {
        return _checkWrite(addr, false);
    }

    uint8_t _checkCRead(uint16_t addr) {
        return _checkRead(addr, false);
    }

    bool _checkWrite(uint16_t addr, bool wordAligned = true) {
        
        uint8_t d = wordAligned ? 2 : 1;
        
        if(wordAligned && !_checkWordAligned(addr)) {
            return false;
        }
        
        if(addr >= _romStart && addr <= _romEnd - d) {
            if(_romWrite != NULL) {
                _romWrite(addr);
            }
            return false;
        }
        
        if(addr < _ramStart || addr >= _ramEnd) {
            if(_undefinedAccess != NULL) {
                _undefinedAccess(addr);
            }
            return false;
        }

        return true;
    }

    uint8_t _checkRead(uint16_t addr, bool wordAligned = true) {
        
        uint8_t d = wordAligned ? 2 : 1;

        if(wordAligned && !_checkWordAligned(addr)) {
            return MEMORY_ACCESS_INVALID;
        }
        
        if(addr >= _romStart && addr <= _romEnd - d) {
            return MEMORY_ACCESS_ROM;
        }
        
        if(addr >= _ramStart || addr <= _ramEnd - d) {
            return MEMORY_ACCESS_RAM;
        }

        if(_undefinedAccess != NULL) {
            _undefinedAccess(addr);

        }

        return MEMORY_ACCESS_INVALID;
    }

    bool _checkWordAligned(uint16_t addr) {
        if((addr & 0x0001) != 0) {
            if(_unalignedWrite != NULL ) {
                _unalignedWrite(addr);
            }
            return false;
        }

        return true;
    }

};

#endif