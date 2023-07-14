#ifndef UKMAKER_FORTHVM_H
#define UKMAKER_FORTHVM_H

#include "Memory.h"
#include "FArduino.h"
#include "ForthIS.h"

class ForthVM; 

typedef void (*Syscall)(ForthVM *vm);

class ForthVM
{

public:
    ForthVM(Memory *ram, Syscall *syscalls, size_t numSyscalls)
    :
    _ram(ram), _syscalls(syscalls), _numSyscalls(numSyscalls)
    {
        _halted = true;
    }

    ~ForthVM() {}

    bool halted() {
        return _halted;
    }

    void reset() {
        _regs[REG_PC] = 0;
        _halted = false;
    }

    void step() {
        _clock();
    }

    void run() {
        while(!halted()) step();
    }

    void load(uint16_t op_cc, uint16_t cc_apply, uint16_t op, uint16_t arga, uint16_t argb) {
        uint16_t instruction = 
            (op_cc << CC_BITS)
            + (cc_apply << CC_APPLY_BIT)
            + (op << OP_BITS)
            + ((arga & 0x0f) << ARGA_BITS)
            + ((argb & 0x0f) << ARGB_BITS);

        _ram->put(_regs[REG_PC], instruction);
        _regs[REG_PC]+=2;
    }

    void load(uint16_t op_cc, uint16_t cc_apply, uint16_t op, uint16_t imm) {
        uint16_t instruction =
         (op_cc << CC_BITS)
         + (cc_apply << CC_APPLY_BIT)
         + (op << OP_BITS)
         + ((imm & 0xff) << ARGB_BITS);
        _ram->put(_regs[REG_PC], instruction);
        _regs[REG_PC]+=2;
    }

    void load(uint16_t imm) {
         _ram->put(_regs[REG_PC], imm);
        _regs[REG_PC]+=2;       
    }

    void load(char c, bool inc) {
        uint16_t v = _ram->get(_regs[REG_PC]);
        // if inc then this is the high byte and we increment to the next word
        if(inc) {
            v = v & 0xff;
            v = v | ((uint16_t)c << 8);
            load(v);
        } else {
            v = v & 0xff00;
            v = v | c;
            _ram->put(_regs[REG_PC], v);
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
        return _regs[REG_PC];
    }

    uint16_t get(uint8_t reg) {
        return _regs[reg];
    }

    void set(uint8_t reg, uint16_t v) {
        _regs[reg] = v;
    }

    bool getC() {
        return _c;
    }

    bool getZ() {
        return _z;
    }

    bool getO() {
        return _odd;
    }

    bool getS() {
        return _sign;
    }

    /***/

    /**
     * Methods for the syscall interface
     **/
    void addSyscall(uint16_t idx, Syscall syscall) {
        _syscalls[idx] = syscall;
    }

    void push(uint16_t c) {
        _ram->put(_regs[REG_SP], (uint16_t)c);
        _regs[REG_SP]-=2;
    }

    uint16_t pop() {
        _regs[REG_SP]+=2;
        return _ram->get(_regs[REG_SP]);
    }

    uint8_t readByte(uint16_t addr) {
        return _ram->getC(addr);
    }

    uint16_t read(uint16_t addr) {
        return _ram->get(addr);
    }

    Memory *ram() {
        return _ram;
    }

    protected:

    bool _halted;

    Memory *_ram;
    Syscall *_syscalls;
    uint8_t _numSyscalls;

    // Registers
    uint16_t _regs[16];

    // flags
    bool _z;
    bool _c;
    bool _sign;
    bool _odd;

    // Run one processor cycle
    void _clock()
    {
        if(_halted) {
            return;
        }

        bool ccapply = false;
        bool ccinvert = false;
        uint8_t cc = 0;
        bool skip = false;

        uint16_t instr = _ram->get(_regs[REG_PC]);
        // Decode the opcode
        uint8_t op = (instr & OP_MASK) >> OP_BITS;
        if((instr & JP_OR_CALL_MASK) != 0) {
            ccapply = (instr & CC_APPLY_MASK) >> CC_APPLY_BIT;
            ccinvert = (instr & CC_INV_MASK) >> CC_INV_BIT;
            cc = (instr & CC_MASK) >> CC_BITS;
            op = (instr & JP_OR_CALL_OP_MASK) >> OP_BITS;
            if(ccapply) {

                switch (cc)
                {
                    case COND_C:
                        skip = !_c;
                        break;
                    case COND_Z:
                        skip = !_z;
                        break;
                    case COND_M:
                        skip = !_sign;
                        break;
                    case COND_P:
                        skip = !_odd;
                        break;
                    default:
                        break;
                }

                if(ccinvert) skip = !skip;
            }
        }

        uint8_t arga = (instr & ARGA_MASK) >> ARGA_BITS;
        uint8_t argb = (instr & ARGB_MASK) >> ARGB_BITS;
        uint8_t u4 = argb;
        int8_t n4 = _sex(argb, 3);
        int8_t n8 = _sex((arga << 4) + argb, 7);

         _regs[REG_PC]+=2;

        switch (op)
        {
            case OP_NOP: break;
            
            case OP_MOV:   
                _regs[arga] = _regs[argb]; 
                break; // Ra <- Rb

            case OP_MOVI:  
                _regs[arga] = n4; 
                break;         // Ra <- #num4

            case OP_MOVIL:
                _regs[arga] = getIL();
                break;

            case OP_MOVAI:  
                _regs[REG_A] = n8; 
                break;         // Ra <- #num6

            case OP_MOVBI:  
                _regs[REG_B] = n8; 
                break;         // Ra <- #num6

            case OP_LD:
                _regs[arga] = _ram->get(_regs[argb]);
                break;

            case OP_LD_B:
                _regs[arga] = _ram->getC(_regs[argb]);
                break;

            case OP_LDAX:
                _regs[REG_A] = _ram->get(_regs[arga] + (n4 << 1));
                break;

            case OP_LDBX:
                _regs[REG_B] = _ram->get(_regs[arga] + (n4 << 1));
                break;

            case OP_LDAX_B:
                _regs[REG_A] = _ram->getC(_regs[arga] + (n4 << 1));
                break;

            case OP_LDBX_B:
                _regs[REG_B] = _ram->getC(_regs[arga] + (n4 << 1));
                break;

            case OP_ST:
                _ram->put(_regs[arga], _regs[argb]);
                break; // (Ra) <- Rb

           case OP_ST_B:
                _ram->putC(_regs[arga], _regs[argb]);
                break; // (Ra) <- Rb

            case OP_STI:
                _ram->put(_regs[arga], n4);
                break; // (Ra) <- #num3

            case OP_STAI:
                _ram->put(_regs[REG_A], n8);
                break; // (Ra) <- #num6

           case OP_STBI:
                _ram->putC(_regs[REG_B], n8);
                break; // (Ra) <- #num6

            case OP_STIL:
                _ram->put(arga, getIL());
                break;

            case OP_STI_B:
                _ram->putC(_regs[arga], n4);
                break; // (Ra) <- #num3

            case OP_STAI_B:
                _ram->putC(_regs[REG_A], n8);
                break; // (Ra) <- #num6

           case OP_STBI_B:
                _ram->putC(_regs[REG_B], n8);
                break; // (Ra) <- #num6

            case OP_STXA:
                _ram->put(_regs[arga] + (n4 << 1), _regs[REG_A]);
                break;

            case OP_STXB:
                _ram->put(_regs[arga] + (n4 << 1), _regs[REG_B]);
                break;

            case OP_STXA_B:
                _ram->putC(_regs[arga] + n4, _regs[REG_A]);
                break;

            case OP_STXB_B:
                _ram->putC(_regs[arga] + n4, _regs[REG_B]);
                break;

            case OP_PUSHD:
                _ram->put(_regs[REG_SP], _regs[arga]);
                _regs[REG_SP]-=2;
                break; // DSTACK <- Ra, INC SP

            case OP_PUSHR:
                _ram->put(_regs[REG_RS], _regs[arga]);
                _regs[REG_RS]-=2;
                break; // RSTACK <- Ra, INC RS

            case OP_POPD:
                _regs[REG_SP]+=2;
                _regs[arga] = _ram->get(_regs[REG_SP]);
                break; // DEC SP, Ra <- DSTACK

            case OP_POPR:
                _regs[REG_RS]+=2;
                _regs[arga] = _ram->get(_regs[REG_RS]);
                break; // DEC RS, Ra <- RSTACK

            case OP_ADD: _add(arga, argb); break;
            case OP_ADDI: _addi(arga, n4); break;
            case OP_ADDAI: _addi(REG_A, n8); break;
            case OP_ADDBI: _addi(REG_B, n8); break;
            case OP_ADDIL: _addl(arga, getIL()); break;

            case OP_CMP: _cmp(arga, argb); break;
            case OP_CMPI: _cmpi(arga, n4); break;
            case OP_CMPAI: _cmpi(REG_A, n8); break;
            case OP_CMPBI: _cmpi(REG_B, n8); break;
            case OP_CMPIL: _cmpl(arga, getIL()); break;

            case OP_SUB: _sub(arga, argb); break;
            case OP_SUBI: _subi4(arga, n4); break;
            case OP_SUBAI: _subi(REG_A, n8); break;
            case OP_SUBBI: _subi(REG_B, n8); break;
            case OP_SUBIL: _subl(arga, getIL()); break;
            
            case OP_MUL: _mul(arga, argb); break;
            case OP_DIV: _div(arga, argb); break;
            case OP_AND: _and(arga, argb); break;
            case OP_OR: _or(arga, argb); break;
            case OP_NOT: _not(arga); break;
            case OP_XOR: _xor(arga, argb); break;

            case OP_SLI: _sl(arga, u4); break;
            case OP_SRI: _sr(arga, u4); break;

            case OP_RLI: _rl(arga, u4); break;
            case OP_RRI: _rr(arga, u4); break;
            case OP_RLCI: _rlc(arga, u4); break;
            case OP_RRCI: _rrc(arga, u4); break;

            case OP_BITI: _bit(arga, u4); break;
            case OP_SETI: _set(arga, u4); break;
            case OP_CLRI: _clr(arga, u4); break;

            case OP_SL: _sl(arga, _regs[argb]); break;
            case OP_SR: _sr(arga, _regs[argb]); break;

            case OP_RL: _rl(arga, _regs[argb]); break;
            case OP_RR: _rr(arga, _regs[argb]); break;
            case OP_RLC: _rlc(arga, _regs[argb]); break;
            case OP_RRC: _rrc(arga, _regs[argb]); break;

            case OP_BIT: _bit(arga, _regs[argb]); break;
            case OP_SET: _set(arga, _regs[argb]); break;
            case OP_CLR: _clr(arga, _regs[argb]); break;

            case OP_JR:
                if(!skip) _regs[REG_PC] = _regs[REG_PC] + (n8 << 1);
                break;  // PC <- PC + #n6

            case OP_JP:
                if(!skip) _regs[REG_PC] = _ram->get(_regs[REG_PC]);
                break; // PC <- #num16

            case OP_JX:
                if(!skip) _regs[REG_PC] = _regs[arga] + (n4 << 1);
                break; // PC <- Ra + #num16

            case OP_JXL:
                if(!skip) _regs[REG_PC] = _regs[arga] + getIL();
                break;

            case OP_CALL: // PC <- #num16
                if(!skip)  {
                    _regs[REG_PC] = _ram->get(_regs[REG_PC]);
                    _regs[REG_PC]+=2;
                    _ram->put(_regs[REG_RS], _regs[REG_PC]);
                    _regs[REG_RS]+=2;
                }
                break;

            case OP_CALLR:
                if(!skip)  {
                    _ram->put(_regs[REG_RS], _regs[REG_PC]);
                    _regs[REG_PC] = _regs[REG_PC] + (n8 << 1);
                }
                break; // PC <- PC + #num6

           case OP_CALLX:
                if(!skip) {
                    _ram->put(_regs[REG_RS], _regs[REG_PC]);
                    _regs[REG_PC] = _regs[arga] + (n8 << 1);
                }
                break; // PC <- PC + #num6

           case OP_CALLXL:
                if(!skip)  {
                    _ram->put(_regs[REG_RS], _regs[REG_PC]);
                    _regs[REG_PC] = _regs[arga] + getIL();
                }
                break; // PC <- PC + #num6

            case OP_RET:
                if(!skip)  {
                    _regs[REG_RS]-=2;
                    _regs[REG_PC] = _ram->get(_regs[REG_RS]);
                }
                break;

            case OP_SYSCALL:
                if(n8 < _numSyscalls && _syscalls[n8] != NULL) {
                    _syscalls[n8](this);
                }// call a high-level routine <call.6> 

                break;

            case OP_HALT:
                _halted = true;
                break;

            default: break; // oops
        }
            
    }

    void _add(uint8_t a, uint8_t b) {
        uint32_t r = (uint32_t)_regs[a] + (uint32_t)_regs[b];
        _regs[a] = r & 0xffff;
        _arithmeticFlags(r);
    }

    void _addi(uint8_t a, int8_t n) {
        uint32_t r = (uint32_t)_regs[a] + n;
        _regs[a] = r & 0xffff;
        _arithmeticFlags(r);
    }

    void _addl(uint8_t a, uint16_t n) {
        uint32_t r = (uint32_t)_regs[a] + n;
        _regs[a] = r & 0xffff;
        _arithmeticFlags(r);
    }

    void _cmp(uint8_t a, uint8_t b) {
        uint32_t r = (uint32_t)_regs[a] - (uint32_t)_regs[b];
        _arithmeticFlags(r);
    }

    void _cmpi(uint8_t a, uint8_t n) {
        uint32_t r = (uint32_t)_regs[a] - n;
        _arithmeticFlags(r);
    }

    void _cmpl(uint8_t a, uint16_t n) {
        uint32_t r = (uint32_t)_regs[a] - n;
        _arithmeticFlags(r);
    }

    void _sub(uint8_t a, uint8_t b) {
        uint32_t r = (uint32_t)_regs[a] - (uint32_t)_regs[b];
        _regs[a] = r & 0xffff;
        _arithmeticFlags(r);
    }

    void _subi(uint8_t a, int8_t n) {
        uint32_t r = (uint32_t)_regs[a] - n;
        _regs[a] = r & 0xffff;
        _arithmeticFlags(r);
    }

    void _subi4(uint8_t a, int8_t n) {
        uint32_t r = (uint32_t)_regs[a] - n;
        _regs[a] = r & 0xffff;
        _arithmeticFlags(r);
    }

    void _subl(uint8_t a, uint16_t n) {
        uint32_t r = (uint32_t)_regs[a] - n;
        _regs[a] = r & 0xffff;
        _arithmeticFlags(r);
    }

   void _mul(uint8_t a, uint8_t b) {
        uint32_t r = (uint32_t)_regs[a] * (uint32_t)_regs[b];
        _regs[a] = r & 0xffff;
        _arithmeticFlags(r);
    }

  void _div(uint8_t a, uint8_t b) {
        uint16_t r = (uint32_t)_regs[a] / (uint32_t)_regs[b];
        _regs[a] = r;
        _arithmeticFlags(r);
    }

    void _and(uint8_t a, uint8_t b) {
        uint16_t r = _regs[a] & _regs[b];
        _regs[a] = r;
        _booleanFlags(r);
    }

    void _or(uint8_t a, uint8_t b) {
        uint16_t r = _regs[a] | _regs[b];
        _regs[a] = r;
        _booleanFlags(r);
    }

    void _not(uint8_t a) {
        uint16_t r = ~_regs[a];
        _regs[a] = r;
        _booleanFlags(r);
    }

    void _xor(uint8_t a, uint8_t b) {
        uint16_t r = _regs[a] ^ _regs[b];
        _regs[a] = r;
        _booleanFlags(r);
    }

    void _sl(uint8_t a, uint8_t n) {
        uint32_t r = _regs[a] << n;
        _regs[a] = r & 0xffff;
        _booleanFlags(r);
    }

    void _sr(uint8_t a, uint8_t n) {
        uint16_t r = _regs[a] >> n;
        bool c = _regs[a] & 1;
        _regs[a] = r;
        _booleanFlags(r);
        _c = c;
    }

    void _rr(uint8_t a, uint8_t n) {
        uint16_t mask = ~((0xffff >> n) << n);
        uint16_t bits = _regs[a] & mask;
        _regs[a] = (_regs[a] >> n) | (bits << (16-n));
        _booleanFlags(_regs[a]);
        _c = (_regs[a] & 0x8000) != 0;
    }

    void _rl(uint8_t a, uint8_t n) {
        uint16_t mask = ~(0xffff >> n);
        uint16_t bits = _regs[a] & mask;
        _regs[a] = (_regs[a] << n) | (bits >> (16-n));
 
        _booleanFlags(_regs[a]);
        _c = (_regs[a] & 0x01) != 0;
    }

    void _rrc(uint8_t a, uint8_t n) {
        for(uint8_t i=0; i < n; i++) {
            uint8_t oldc = _c ? 0x8000 : 0;
            _c = (_regs[a] & 0x01) != 0;
            _regs[a] = (_regs[a] >> 1) | oldc; 
        }       
    }

    void _rlc(uint8_t a, uint8_t n) {
        for(uint8_t i=0; i < n; i++) {
            uint8_t oldc = _c ? 0x01 : 0;
            _c = (_regs[a] & 0x8000) != 0;
            _regs[a] = (_regs[a] << 1) | oldc; 
        }       
    }

    void _booleanFlags(uint32_t v) {
        _c = (v & (uint32_t)0x10000) == (uint32_t)0x10000;
        _z = v == 0;
        _odd = v & 0x01;
        _sign = (v & (uint32_t)0x08000) != 0;
    }

    void _arithmeticFlags(uint32_t v) {
        _c = (v & (uint32_t)0x10000) == (uint32_t)0x10000;
        _z = v == 0;
        // odd(P) functions as overflow
        _odd = (v & (uint32_t)0xffff0000) != 0;
        _sign = (v & (uint32_t)0x08000) != 0;
    }

    /*
    * Sign-extend n and return a signed number
    * b is the sign bit index. Make the number n a signed 8-bit number
    */
    int8_t _sex(uint8_t n, uint8_t b) {
        
        if(n & (1 << b)) {
            uint8_t bits = (0xff >> b) << b;
            return n | bits;
        }

        return n;
    }

    uint16_t getIL() {
        uint16_t v = _ram->get(_regs[REG_PC]);
        _regs[REG_PC] += 2;
        return v;
    }

    void _bit(uint8_t arga, uint8_t n) {
        _z = (_regs[arga] & (1 << n)) == 0;
    }

    void _set(uint8_t arga, uint8_t n) {
        _regs[arga] = _regs[arga] | (1 << n);
    }

    void _clr(uint8_t arga, uint8_t n) {
        _regs[arga] = _regs[arga] & ~(1 << n);
    }
};
#endif