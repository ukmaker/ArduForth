#ifndef UKMAKER_LOADER_H
#define UKMAKER_LOADER_H

#include "../runtime/ArduForth/ForthVM.h"

class Loader {

    public:

    Loader(Memory *ram) : _ram(ram), _pc(0) {}
    ~Loader() {}

    void reset() {
        _pc = 0;
    }

    void load(uint16_t op_cc, uint16_t cc_apply, uint16_t op, uint16_t arga, uint16_t argb) {
        uint16_t instruction = 
            (op_cc << CC_BITS)
            + (cc_apply << CC_APPLY_BIT)
            + (op << OP_BITS)
            + ((arga & 0x0f) << ARGA_BITS)
            + ((argb & 0x0f) << ARGB_BITS);

        _ram->put(_pc, instruction);
        _pc += 2;
    }

    void load(uint16_t op_cc, uint16_t cc_apply, uint16_t op, uint16_t imm) {
        uint16_t instruction =
         (op_cc << CC_BITS)
         + (cc_apply << CC_APPLY_BIT)
         + (op << OP_BITS)
         + ((imm & 0xff) << ARGB_BITS);
        _ram->put(_pc, instruction);
        _pc += 2;
    }

    void load(uint16_t imm) {
         _ram->put(_pc, imm);
        _pc += 2;       
    }

    void load(char c, bool inc) {
        uint16_t v = _ram->get(_pc);
        // if inc then this is the high byte and we increment to the next word
        if(inc) {
            v = v & 0xff;
            v = v | ((uint16_t)c << 8);
            load(v);
        } else {
            v = v & 0xff00;
            v = v | c;
            _ram->put(_pc, v);
        }
    }

    uint16_t load(const char *str) {
        char c;
        bool h = false;
        while((c = *str++) != 0) {
            load(c, h);
            h = !h;
        }
        // make sure we're aligned on a word boundary
        if(h) load('\0', h);
        return _pc;
    }

    protected:

    Memory *_ram;
    uint32_t _pc;

};

#endif