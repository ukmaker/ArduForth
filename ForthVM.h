#ifndef UKMAKER_FORTHVM_H
#define UKMAKER_FORTHVM_H

#include "RAM.h"
#include "Arduino.h"
#include "ForthIS.h"

class ForthVM; 

typedef void (*Syscall)(ForthVM *vm);

class ForthVM
{

public:
    ForthVM(RAM &ram, Syscall *syscalls, size_t numSyscalls)
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

    void load(uint16_t op_cc, uint16_t cc_apply, uint16_t op, uint16_t arga, uint16_t argb) {
        uint16_t instruction = 
            (op_cc << CC_BITS)
            + (cc_apply << CC_APPLY_BITS)
            + (op << OP_BITS)
            + ((arga & 0x07) << ARGA_BITS)
            + ((argb & 0x07) << ARGB_BITS);

        _ram.put(_regs[REG_PC], instruction);
        _regs[REG_PC]+=2;
    }

    void load(uint16_t op_cc, uint16_t cc_apply, uint16_t op, uint16_t imm) {
        uint16_t instruction =
         (op_cc << CC_BITS)
         + (cc_apply << CC_APPLY_BITS)
         + (op << OP_BITS)
         + ((imm & 0x3f) << ARGB_BITS);
        _ram.put(_regs[REG_PC], instruction);
        _regs[REG_PC]+=2;
    }

    void load(uint16_t imm) {
         _ram.put(_regs[REG_PC], imm);
        _regs[REG_PC]+=2;       
    }

    void load(char c, bool inc) {
        uint16_t v = _ram.get(_regs[REG_PC]);
        // if inc then this is the high byte and we increment to the next word
        if(inc) {
            v = v & 0xff;
            v = v | ((uint16_t)c << 8);
            load(v);
        } else {
            v = v & 0xff00;
            v = v | c;
            _ram.put(_regs[REG_PC], v);
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

    void push(char c) {
        _ram.put(_regs[REG_SP], (uint16_t)c);
        _regs[REG_SP]+=2;
    }

    uint16_t pop() {
        _regs[REG_SP]-=2;
        return _ram.get(_regs[REG_SP]);
    }

    uint8_t readByte(uint16_t addr) {
        return _ram.getByte(addr);
    }

    uint16_t read(uint16_t addr) {
        return _ram.get(addr);
    }

    void setDebugging(bool d) {
        _debugging = d;
    }

    protected:

    bool _halted;
    bool _debugging = false;


    RAM &_ram;
    Syscall *_syscalls;
    size_t _numSyscalls;

    // The inner interpreter. This is the key to everything
    //
    // Registers
    uint16_t _regs[8];

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

        uint16_t instr = _ram.get(_regs[REG_PC]);
        // Decode the opcode
        uint8_t cc = (instr & CC_MASK) >> CC_BITS;
        uint8_t ccapply = (instr & CC_APPLY_MASK) >> CC_APPLY_BITS;
        uint8_t op = (instr & OP_MASK) >> OP_BITS;
        uint8_t arga = (instr & ARGA_MASK) >> ARGA_BITS;
        uint8_t argb = (instr & ARGB_MASK) >> ARGB_BITS;
        int8_t n3 = _sex(argb, 2);
        int8_t n6 = _sex((arga << 3) + argb, 5);
        _regs[REG_PC]+=2;

        if(_debugging) {
            _debug(cc, ccapply, op, n3, n6, arga, argb);
        }

        bool skip = false;
        if(ccapply == COND_APPLY_MATCH || ccapply == COND_APPLY_NOT_MATCH) {

            bool match = (ccapply == COND_APPLY_NOT_MATCH);

            switch (cc)
            {
                case COND_C:
                    skip = _c;
                    break;
                case COND_Z:
                    skip = _z;
                    break;
                case COND_M:
                    skip = _sign;
                    break;
                case COND_P:
                    skip = _odd;
                    break;
                default:
                    break;
            }

            if(skip == match) {
                skip = true;
            } else {
                skip = false;
            }
        }

        // Treat L instructions specially in case we need to skip two words
        switch(op) {
            case OP_LDL:            
            case OP_STL: 
            case OP_ADDL: 
            case OP_CMPL: 
            case OP_JP:
            case OP_JPIDX:
            case OP_CALL: 
                if(skip) _regs[REG_PC]+=2;
                break;
            default: break;
        }

        if(skip) op = OP_NOP;

        switch (op)
        {
            case OP_NOP: break;
            
            case OP_LD:   
                _regs[arga] = _regs[argb]; 
                break; // Ra <- Rb

            case OP_LDI:  
                _regs[arga] = n3; 
                break;         // Ra <- #num3

            case OP_LDAI:  
                _regs[REG_A] = n6; 
                break;         // Ra <- #num6

            case OP_LDBI:  
                _regs[REG_B] = n6; 
                break;         // Ra <- #num6

            case OP_LDL:  
                _regs[arga] = _ram.get(_regs[REG_PC]); // Ra <- #num16
                _regs[REG_PC]+=2;
                break;

            case OP_FETCH:
                _regs[arga] =  _ram.get(_regs[argb]);
                break; // RA <- (Rb)

            case OP_ST:
                _ram.put(_regs[arga], _regs[argb]);
                break; // (Ra) <- Rb

            case OP_STI:
                _ram.put(_regs[arga], n3);
                break; // (Ra) <- #num3

            case OP_STAI:
                _ram.put(_regs[REG_A], n6);
                break; // (Ra) <- #num6

           case OP_STAIB:
                _ram.putByte(_regs[REG_A], n6);
                break; // (Ra) <- #num6

            case OP_STAIX:
                _ram.put(_regs[arga] + _regs[argb] , _regs[REG_A]);
                break; // (Ra+Rb) <- RA

            case OP_STAIXB:
                _ram.putByte(_regs[arga] + _regs[argb] , _regs[REG_A]);
                break; // (Ra+Rb) <- RA

            case OP_STBI:
                _ram.put(_regs[REG_B], n6);
                break; // (Ra) <- #num6

            case OP_STBIB:
                _ram.putByte(_regs[REG_B], n6);
                break; // (Ra) <- #num6

            case OP_STBIX:
                _ram.put(_regs[arga] + _regs[argb] , _regs[REG_B]);
                break; // (Ra+Rb) <- RB

            case OP_STBIXB:
                _ram.putByte(_regs[arga] + _regs[argb] , _regs[REG_B]);
                break; // (Ra+Rb) <- RB

            case OP_STL:
                _ram.put(_regs[arga], _ram.get(_regs[REG_PC]));
                _regs[REG_PC]+=2;
                break; // (Ra) <- #num16

            case OP_PUSHD:
                _ram.put(_regs[REG_SP], _regs[arga]);
                _regs[REG_SP]+=2;
                break; // DSTACK <- Ra, INC SP

            case OP_PUSHR:
                    _ram.put(_regs[REG_RS], _regs[arga]);
                _regs[REG_RS]+=2;
                break; // RSTACK <- Ra, INC RS

            case OP_POPD:
                _regs[REG_SP]-=2;
                _regs[arga] = _ram.get(_regs[REG_SP]);
                break; // DEC SP, Ra <- DSTACK

            case OP_POPR:
                _regs[REG_RS]-=2;
                _regs[arga] = _ram.get(_regs[REG_RS]);
                break; // DEC RS, Ra <- RSTACK

            case OP_ADD: _add(arga, argb); break;
            case OP_ADDI: _addi(arga, n3); break;
            case OP_ADDAI: _addi(REG_A, n3); break;
            case OP_ADDBI: _addi(REG_B, n3); break;
            case OP_ADDL: _addl(arga, _ram.get(_regs[REG_PC])); _regs[REG_PC]+=2; break;

            case OP_CMP: _cmp(arga, argb); break;
            case OP_CMPI: _cmpi(arga, n3); break;
            case OP_CMPAI: _cmpi(REG_A, n6); break;
            case OP_CMPBI: _cmpi(REG_B, n6); break;
            case OP_CMPL: _cmpl(arga, _ram.get(_regs[REG_PC])); _regs[REG_PC]+=2; break;

            case OP_SUB: _sub(arga, argb); break;
            case OP_MUL: _mul(arga, argb); break;
            case OP_DIV: _div(arga, argb); break;
            case OP_AND: _and(arga, argb); break;
            case OP_OR: _or(arga, argb); break;
            case OP_NOT: _not(arga); break;
            case OP_XOR: _xor(arga, argb); break;
            case OP_SL: _sl(arga); break;
            case OP_SR: _sr(arga); break;

            case OP_JR:
                _regs[REG_PC] = _regs[REG_PC] + (n6 << 1);
                break;  // PC <- PC + #n6

            case OP_JP:
                _regs[REG_PC] = _ram.get(_regs[REG_PC]);
                break; // PC <- #num16

            case OP_JPIDX:
                _regs[REG_PC] = _regs[arga] + _ram.get(_regs[REG_PC]);
                break; // PC <- Ra + #num16

            case OP_CALL: // PC <- #num16
                _regs[REG_PC] = _ram.get(_regs[REG_PC]);
                _regs[REG_PC]+=2;
                _ram.put(_regs[REG_RS], _regs[REG_PC]);
                _regs[REG_RS]+=2;
                break;

            case OP_CALLR:
                _ram.put(_regs[REG_RS], _regs[REG_PC]);
                _regs[REG_PC] = _regs[REG_PC] + (n6 << 1);
                break; // PC <- PC + #num6

            case OP_RET:
                _regs[REG_RS]-=2;
                _regs[REG_PC] = _ram.get(_regs[REG_RS]);
                break;

            case OP_SYSCALL:

                if(_syscalls[n6] != NULL) {
                    _syscalls[n6](this);
                }// call a high-level routine <call.6> 

                break;

            case OP_HALT:
                _halted = true;
                break;

            default: break; // oops
        }
            
    }

    void _add(uint8_t a, uint8_t b) {
        uint32_t r = _regs[a] + _regs[b];
        _regs[a] = r & 0xffff;
        _flags(r);
    }

    void _addi(uint8_t a, int8_t n) {
        uint32_t r = _regs[a] + n;
        _regs[a] = r & 0xffff;
        _flags(r);
    }

    void _addl(uint8_t a, uint16_t n) {
        uint32_t r = _regs[a] + n;
        _regs[a] = r & 0xffff;
        _flags(r);
    }

    void _cmp(uint8_t a, uint8_t b) {
        uint32_t r = _regs[a] - _regs[b];
        _c = _regs[a] < _regs[b];
        _z = r == 0;
    }

    void _cmpi(uint8_t a, uint8_t n) {
        uint32_t r = _regs[a] - n;
        _flags(r);
    }

    void _cmpl(uint8_t a, uint16_t n) {
        uint32_t r = _regs[a] - n;
        _flags(r);
    }

    void _sub(uint8_t a, uint8_t b) {
        uint32_t r = _regs[a] - _regs[b];
        _regs[a] = r & 0xffff;
        _flags(r);
    }

   void _mul(uint8_t a, uint8_t b) {
        uint32_t r = _regs[a] * _regs[b];
        _regs[a] = r & 0xffff;
        _flags(r);
    }

  void _div(uint8_t a, uint8_t b) {
        uint16_t r = _regs[a] / _regs[b];
        _regs[a] = r;
        _flags(r);
    }

    void _and(uint8_t a, uint8_t b) {
        uint16_t r = _regs[a] & _regs[b];
        _regs[a] = r;
        _flags(r);
    }

    void _or(uint8_t a, uint8_t b) {
        uint16_t r = _regs[a] | _regs[b];
        _regs[a] = r;
        _flags(r);
    }

    void _not(uint8_t a) {
        uint16_t r = ~_regs[a];
        _regs[a] = r;
        _flags(r);
    }

    void _xor(uint8_t a, uint8_t b) {
        uint16_t r = _regs[a] ^ _regs[b];
        _regs[a] = r;
        _flags(r);
    }

    void _sl(uint8_t a) {
        uint32_t r = _regs[a] << 1;
        _regs[a] = r & 0xffff;
        _flags(r);
    }

    void _sr(uint8_t a) {
        uint16_t r = _regs[a] >> 1;
        bool c = _regs[a] & 1;
        _regs[a] = r;
        _flags(r);
        _c = c;
    }

    void _flags(uint32_t v) {
        _c = (v & 0x10000) == 0x10000;
        _z = v == 0;
        _odd = v & 0x01;
        _sign = v & 0x08000;
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

    void _printarg(uint8_t arg) {
        switch(arg) {
            case REG_A: printf("A"); break;
            case REG_B: printf("B"); break;
            case REG_CA: printf("CA"); break;
            case REG_I: printf("I"); break;
            case REG_PC: printf("PC"); break;
            case REG_RS: printf("RS"); break;
            case REG_SP: printf("SP"); break;
            case REG_WA: printf("WA"); break;
            default: break;
        }
    }
    void _printargi(uint8_t arg) {
        printf("(");
        _printarg(arg);
        printf(")");
    }
    void _printargs(uint8_t arga, uint8_t argb) {
        _printarg(arga);
        printf(",");
        _printarg(argb);
        printf("\n");
    }

    void _debug(uint8_t cc, uint8_t ccapply, uint8_t op, int8_t n3, int8_t n6, uint8_t arga, uint8_t argb) {
        // PC [ S O Z C ] CC OP ARGS 
        printf("%04x - %04x D[ %04x ] R[ %04x] [ S%d O%d Z%d C%d ] [ A:%04x B:%04x PC:%04x WA:%04x SP:%04x RS:%04x CA:%04x I:%04x ]", 
        _regs[REG_PC]-2, 
        _ram.get(_regs[REG_PC]-2),
        _ram.get(_regs[REG_SP]-2),
        _ram.get(_regs[REG_RS]-2),
        _sign, _odd, _z, _c,
        _regs[0],
        _regs[1],
        _regs[2],
        _regs[3],
        _regs[4],
        _regs[5],
        _regs[6],
        _regs[7]
        );

        if(ccapply == COND_APPLY_MATCH || ccapply == COND_APPLY_NOT_MATCH) {
            if(ccapply == COND_APPLY_NOT_MATCH) {
                printf("!");
            }
            switch(cc) {
                case COND_C: printf("C "); break;
                case COND_Z: printf("Z "); break;
                case COND_M: printf("M "); break;
                case COND_P: printf("P "); break;
                default: break;
            }
        }

        switch(op) {
            case OP_NOP: printf("NOP"); break;
            case OP_LD: printf("LD "); _printargs(arga, argb); break;    // Ra <- Rb
            case OP_LDI: printf("LDI "); _printarg(arga); printf(",%02x", n3); break;     // Ra <- #num3 - load tiny numbers
            case OP_LDAI: printf("LDAI "); printf("%02x\n", n6); break;     // RA <- #num6 - load medium numbers
            case OP_LDBI: printf("LDBI "); printf("%02x\n", n6); break;     // RB <- #num6 - load medium numbers
            case OP_LDL: printf("LDL "); _printarg(arga); printf(",%04x", _ram.get(_regs[REG_PC])); break;   // Ra <- #num16
            case OP_FETCH: printf("FETCH "); _printarg(arga); printf(",");_printargi(argb);  break;// Ra <- (Rb)
            case OP_ST: printf("ST "); _printargi(arga); printf(",");_printarg(argb); break;    // (Ra) <- Rb
            case OP_STI: printf("STI "); _printargi(arga); printf(",");printf("%d", n3);  break;   // (Ra) <- #num3
            case OP_STAI: printf("STAI "); printf("%02x", n3); break;     // (RA) <- #num3
            case OP_STBI: printf("STBI "); printf("%02x", n3);  break;    // (RB) <- #num3
            case OP_STAIX: printf("STAIX "); _printargs(arga, argb); break; 
            case OP_STBIX: printf("STBIX "); _printargs(arga, argb); break;
            case OP_STAIB: printf("STAIB "); printf("%02x", n3); break;     // (RA) <- #num3
            case OP_STBIB: printf("STBIB "); printf("%02x", n3);  break;    // (RB) <- #num3
            case OP_STAIXB: printf("STAIXB "); _printargs(arga, argb); break; 
            case OP_STBIXB: printf("STBIXB "); _printargs(arga, argb); break;
            case OP_STL: printf("STL "); _printargi(arga); printf(",%04x", _ram.get(_regs[REG_PC]));  break;     // (Ra) <- #num16
            case OP_PUSHD: printf("PUSHD "); _printarg(arga); break; // DSTACK <- Ra, INC SP
            case OP_PUSHR: printf("PUSHR "); _printarg(arga); break; 
            case OP_POPD: printf("POPD "); _printarg(arga); break;  // Ra <- DSTACK, DEC SP
            case OP_POPR: printf("POPR "); _printarg(arga); break; 

            case OP_ADD: printf("ADD "); _printargs(arga, argb); break;   
            case OP_ADDI: printf("ADDI "); _printarg(arga); printf("%02x", n3); break;   
            case OP_ADDAI: printf("ADDAI "); printf("%02x", n6);  break;   
            case OP_ADDBI: printf("ADDBI "); printf("%02x", n6);  break;   
            case OP_ADDL: printf("ADDL "); _printarg(arga); printf("%04x", _ram.get(_regs[REG_PC]));  break;   

            case OP_CMP: printf("CMP "); _printargs(arga, argb); break;   
            case OP_CMPI: printf("CMPI "); _printarg(arga); printf("%02x", n3); break;   
            case OP_CMPAI: printf("CMPAI "); printf("%02x", n6);  break;   
            case OP_CMPBI: printf("CMPBI "); printf("%02x", n6);  break;   
            case OP_CMPL: printf("CMPL "); _printarg(arga); printf("%04x", _ram.get(_regs[REG_PC]));  break;   

            case OP_SUB: printf("SUB "); _printargs(arga, argb); break;   
            case OP_MUL: printf("MUL "); _printargs(arga, argb); break;   
            case OP_DIV: printf("DIV "); _printargs(arga, argb); break;   
            case OP_AND: printf("AND "); _printargs(arga, argb); break;   
            case OP_OR: printf("OR "); _printargs(arga, argb); break;   
            case OP_NOT: printf("NOT "); _printarg(arga); break;   
            case OP_XOR: printf("XOR "); _printargs(arga, argb); break;   
            case OP_SL: printf("SL "); _printarg(arga); break;   
            case OP_SR: printf("SR "); _printarg(arga); break;   

            case OP_JR: printf("JR ");  printf("%02x", n6); break; // PC <- PC + #num6 [ equivalent to OP_LDI PC,#num7 ]
            case OP_JP: printf("JP ");  printf("%04x", _ram.get(_regs[REG_PC])); break;  // PC <- #num16
            case OP_JPIDX: printf("JPIDX "); _printarg(arga);  printf(",%04x", _ram.get(_regs[REG_PC])); break; // PC <- Ra + #num16

            case OP_CALL: printf("CALL ");  printf(",%04x", _ram.get(_regs[REG_PC])); break; 
            case OP_CALLR: printf("CALLR ");  printf(",%02x", n6); break; 
            case OP_RET: printf("RET"); break;
            case OP_SYSCALL: printf("SYSCALL %02x", n6); break; // call a high-level routine <call.6>
            case OP_HALT: printf("HALT"); break;
            default: break;      
    }

    printf("\n");
    }

};
#endif