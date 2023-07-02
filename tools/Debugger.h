#ifndef UKMAKER_DEBUGGER_H
#define UKMAKER_DEBUGGER_H

#include "../runtime/ArduForth/ForthVM.h"
#include "Assembler.h"

class Debugger
{

public:
    Debugger() {}
    ~Debugger() {}

    void setAssembler(Assembler *fasm)
    {
        _asm = fasm;
    }

    void setVM(ForthVM *vm)
    {
        _vm = vm;
    }

    void setShowForthWordsOnly()
    {
        _showWords = true;
    }

    void setBump(uint16_t b) 
    {
        _bump = b;
    }

    void setBreakpoint1(uint16_t addr)
    {
        _breakpoint1 = addr;
    }
    void setBreakpoint2(uint16_t addr)
    {
        _breakpoint2 = addr;
    }

    void setLabelBreakpoint1(const char *name)
    {
        int addr = _asm->getLabelAddress(name);
        if (addr != -1)
        {
            setBreakpoint1(addr);
        }
    }

    void setLabelBreakpoint2(const char *name)
    {
        int addr = _asm->getLabelAddress(name);
        if (addr != -1)
        {
            setBreakpoint2(addr);
        }
    }
    
    void printWALabel(uint16_t wa) {
        Token *label = _asm->getLabel(wa);
        if(label != NULL) {
            printf("%s\n", label->name);
        }
    }

    void setVerbose(bool v) {
        _verbose = v;
    }

    void breakpoint()
    {
        if (_vm->get(REG_I) == _breakpoint2)
        {
            printf("1*");
        }
        else if (_vm->get(REG_PC) == _breakpoint1)
        {
            printf("2*");
        }
        else if(_verbose)
        {
            printf("  ");
        }

        if(_bump > 0 && ((_steps % _bump) == 0))
        {
            printf("  ");
        }
        _steps++;
    }

    void reset()
    {
        _vm->reset();
    }

    void step()
    {
        printCurrentStep();
        _vm->step();
    }

    void run()
    {
        while (!_vm->halted())
            step();
    }

    void run(uint16_t steps)
    {
        for (uint16_t i = 0; i < steps; i++)
        {
            if (!_vm->halted())
                step();
        }
    }

    void printCurrentStep()
    {
        uint16_t pc = _vm->get(REG_PC);
        uint16_t instr = _vm->ram()->get(pc);
        // Decode the opcode
        uint8_t cc = (instr & CC_MASK) >> CC_BITS;
        bool ccapply = ((instr & CC_APPLY_MASK) >> CC_APPLY_BIT) != 0;
        bool ccinvert = ((instr & CC_INV_MASK) >> CC_INV_BIT) != 0;
        uint8_t op = (instr & OP_MASK) >> OP_BITS;
        if ((instr & JP_OR_CALL_MASK) != 0)
        {
            op = (instr & JP_OR_CALL_OP_MASK) >> OP_BITS;
        }
        uint8_t arga = (instr & ARGA_MASK) >> ARGA_BITS;
        uint8_t argb = (instr & ARGB_MASK) >> ARGB_BITS;
        int8_t u4 = argb;
        int8_t n4 = _sex(argb, 3);
        int8_t n8 = _sex((arga << 4) + argb, 7);
        uint16_t n16 = _vm->ram()->get(pc + 2);
        const char *label = NULL;
        Token *tok = _asm->getOpcode(pc);
        if (tok != NULL && tok->label != NULL)
        {
            label = tok->label->name;
        }

        if (label != NULL) {
            if(_verbose && _showWords) {
                uint16_t indent = 0x2000 - _vm->get(REG_RS);
                while(indent > 0) {
                    printf(" ");
                    indent--;
                }
            }
            if(_verbose) printf("%s\n", label);
        }

        breakpoint();

        if(!_verbose || _showWords) return;
        printf("%04x - %04x D[%04x] R[%04x] [S%d O%d Z%d C%d] [0:%04x 1:%04x 2:%04x 3:%04x 4:%04x 5:%04x 6:%04x 7:%04x",
               pc,
               _vm->ram()->get(pc),
               _vm->ram()->get(_vm->get(REG_SP) + 2),
               _vm->ram()->get(_vm->get(REG_RS) + 2),
               _vm->getS() ? 1 : 0, _vm->getO() ? 1 : 0, _vm->getZ() ? 1 : 0, _vm->getC() ? 1 : 0,
               _vm->get(REG_0),
               _vm->get(REG_1),
               _vm->get(REG_2),
               _vm->get(REG_3),
               _vm->get(REG_4),
               _vm->get(REG_5),
               _vm->get(REG_6),
               _vm->get(REG_7));

        // PC [ S O Z C ] CC OP ARGS
        printf(" A:%04x B:%04x PC:%04x WA:%04x SP:%04x RS:%04x CA:%04x I:%04x]",
               _vm->get(REG_A),
               _vm->get(REG_B),
               _vm->get(REG_PC),
               _vm->get(REG_WA),
               _vm->get(REG_SP),
               _vm->get(REG_RS),
               _vm->get(REG_CA),
               _vm->get(REG_I));

        switch (op)
        {
        case OP_NOP:
            printf("NOP");
            break;

        case OP_MOV:
            instructionRR("MOV", arga, argb);
            break;
        case OP_MOVI:
            instructionRI4("MOVI", arga, n4);
            break;
        case OP_MOVAI:
            instructionI8("MOVAI", n8);
            break;
        case OP_MOVBI:
            instructionI8("MOVBI", n8);
            break;
        case OP_MOVIL:
            instructionRIL("MOVIL", arga, n16);
            break;

        case OP_LD:
            instructionRR("LD", arga, argb);
            break; // Ra <- Rb
        case OP_LD_B:
            instructionRR("LD_B", arga, argb);
            break;
        case OP_LDAX:
            instructionRI4("LDAX", arga, n4);
            break;
        case OP_LDBX:
            instructionRI4("LDBX", arga, n4);
            break;
        case OP_LDAX_B:
            instructionRI4("LDAX_B", arga, n4);
            break;
        case OP_LDBX_B:
            instructionRI4("LDBX_B", arga, n4);
            break;

        case OP_ST:
            instructionRR("ST", arga, argb);
            break;
        case OP_ST_B:
            instructionRR("ST_B", arga, argb);
            break;
        case OP_STI:
            instructionRI4("STI", arga, n4);
            break;
        case OP_STAI:
            instructionI8("STAI", n8);
            break;
        case OP_STBI:
            instructionI8("STBI", n8);
            break;
        case OP_STIL:
            instructionRIL("STIL", arga, n16);
            break;
        case OP_STI_B:
            instructionRI4("STI_B", arga, n4);
            break;
        case OP_STAI_B:
            instructionI8("STAI_B", n8);
            break;
        case OP_STBI_B:
            instructionI8("STBI_B", n8);
            break;

        case OP_PUSHD:
            printf("PUSHD ");
            _printarg(arga);
            break; // DSTACK <- Ra, INC SP
        case OP_PUSHR:
            printf("PUSHR ");
            _printarg(arga);
            break;
        case OP_POPD:
            printf("POPD ");
            _printarg(arga);
            break; // Ra <- DSTACK, DEC SP
        case OP_POPR:
            printf("POPR ");
            _printarg(arga);
            break;

        case OP_ADD:
            instructionRR("ADD", arga, argb);
            break;
        case OP_ADDI:
            instructionRI4("ADDI", arga, n4);
            break;
        case OP_ADDAI:
            instructionI8("ADDAI", n8);
            break;
        case OP_ADDBI:
            instructionI8("ADDBI", n8);
            break;
        case OP_ADDIL:
            instructionRIL("ADDIL", arga, n16);
            break;

        case OP_CMP:
            instructionRR("CMP", arga, argb);
            break;
        case OP_CMPI:
            instructionRI4("CMPI", arga, n4);
            break;
        case OP_CMPAI:
            instructionI8("CMPAI", n8);
            break;
        case OP_CMPBI:
            instructionI8("CMPBI", n8);
            break;
        case OP_CMPIL:
            instructionRIL("CMPIL", arga, n16);
            break;

        case OP_SUB:
            instructionRR("SUB", arga, argb);
            break;
        case OP_SUBI:
            instructionRI4("SUBI", arga, n4);
            break;
        case OP_SUBAI:
            instructionI8("SUBAI", n8);
            break;
        case OP_SUBBI:
            instructionI8("SUBBI", n8);
            break;
        case OP_SUBIL:
            instructionRIL("SUBIL", arga, n16);
            break;

        case OP_MUL:
            instructionRR("MUL", arga, argb);
            break;
        case OP_DIV:
            instructionRR("DIV", arga, argb);
            break;
        case OP_AND:
            instructionRR("AND", arga, argb);
            break;
        case OP_OR:
            instructionRR("OR", arga, argb);
            break;
        case OP_NOT:
            instructionR("NOT", arga);
            break;
        case OP_XOR:
            instructionRR("XOR", arga, argb);
            break;

        case OP_SL:
            instructionRUI4("SL", arga, u4);
            break;
        case OP_SR:
            instructionRUI4("SR", arga, u4);
            break;
        case OP_RL:
            instructionRUI4("RL", arga, u4);
            break;
        case OP_RR:
            instructionRUI4("RR", arga, u4);
            break;
        case OP_RLC:
            instructionRUI4("RLC", arga, u4);
            break;
        case OP_RRC:
            instructionRUI4("RRC", arga, u4);
            break;

        case OP_BIT:
            instructionRR("BIT", arga, argb);
            break;
        case OP_SET:
            instructionRR("SET", arga, argb);
            break;
        case OP_CLR:
            instructionRR("CLR", arga, argb);
            break;

        case OP_BITI:
            instructionRUI4("BITI", arga, u4);
            break;
        case OP_SETI:
            instructionRUI4("SETI", arga, u4);
            break;
        case OP_CLRI:
            instructionRUI4("CLRI", arga, u4);
            break;

        case OP_RET:
            printf("RET");
            break;
        case OP_SYSCALL:
            printf("SYSCALL %02x", n8);
            break; // call a high-level routine <call.6>
        case OP_HALT:
            printf("HALT");
            break;
        case OP_BRK:
            printf("BRK");
            break;

        case OP_JP:
            jmpIL("JP", ccapply, ccinvert, cc, n16);
            break;
        case OP_JR:
            jmpI8("JR", ccapply, ccinvert, cc, n8);
            break;
        case OP_JX:
            jmpRI4("JX", ccapply, ccinvert, cc, arga, n4);
            break;
        case OP_JXL:
            jmpRIL("JXL", ccapply, ccinvert, cc, arga, n16);
            break;

        case OP_CALL:
            jmpIL("CALL", ccapply, ccinvert, cc, n16);
            break;
        case OP_CALLR:
            jmpI8("CALLR", ccapply, ccinvert, cc, n8);
            break;
        case OP_CALLX:
            jmpRI4("CALLX", ccapply, ccinvert, cc, arga, n4);
            break;
        case OP_CALLXL:
            jmpRIL("CALLXL", ccapply, ccinvert, cc, arga, n16);
            break;

        default:
            break;
        }

        printf("\n");
    }

    void instructionR(const char *instr, uint8_t a)
    {
        printf("%s ", instr);
        _printarg(a);
    }

    void instructionRR(const char *instr, uint8_t a, uint8_t b)
    {
        printf("%s ", instr);
        _printarg(a);
        printf(",");
        _printarg(b);
    }

    void instructionI8(const char *instr, int8_t n8)
    {
        printf("%s %04x", instr, n8);
    }

    void instructionRI4(const char *instr, uint8_t a, int8_t n4)
    {
        printf("%s ", instr);
        _printarg(a);
        printf(",%01x", n4);
    }

    void instructionRI8(const char *instr, uint8_t a, int8_t n8)
    {
        printf("%s ", instr);
        _printarg(a);
        printf(",%02x", n8);
    }

    void instructionRIL(const char *instr, uint8_t a, uint16_t n16)
    {
        printf("%s ", instr);
        _printarg(a);
        printf(",%04x", n16);
    }

    void instructionRUI4(const char *instr, uint8_t a, uint8_t u4)
    {
        printf("%s ", instr);
        _printarg(a);
        printf(",%01x", u4);
    }

    void instructionIL(const char *instr, uint16_t n16)
    {
        printf("%s %04x", instr, n16);
    }

    void condition(bool ccapply, bool ccinvert, uint8_t cc)
    {
        if (ccapply)
        {
            printf("[");
            if (ccinvert)
            {
                printf("N");
            }
            switch (cc)
            {
            case COND_C:
                printf("C");
                break;
            case COND_Z:
                printf("Z");
                break;
            case COND_M:
                printf("M");
                break;
            case COND_P:
                printf("P");
                break;
            default:
                break;
            }
            printf("]");
        }
        printf(" ");
    }

    void jmpI8(const char *inst, bool ccapply, bool ccinvert, uint8_t cc, int8_t n8)
    {
        printf("%s", inst);
        condition(ccapply, ccinvert, cc);
        printf("%02x", n8);
    }

    void jmpIL(const char *inst, bool ccapply, bool ccinvert, uint8_t cc, uint16_t n16)
    {

        printf("%s", inst);
        condition(ccapply, ccinvert, cc);
        printf("%04x", n16);
    }

    void jmpRI4(const char *inst, bool ccapply, bool ccinvert, uint8_t cc, uint8_t a, int8_t n8)
    {

        printf("%s", inst);
        condition(ccapply, ccinvert, cc);
        _printarg(a);
        printf(",%02x", n8);
    }

    void jmpRIL(const char *inst, bool ccapply, bool ccinvert, uint8_t cc, uint8_t a, uint16_t n16)
    {

        printf("%s", inst);
        condition(ccapply, ccinvert, cc);
        _printarg(a);
        printf(",%04x", n16);
    }

protected:
    Assembler *_asm;
    ForthVM *_vm;
    uint16_t _breakpoint1, _breakpoint2;
    bool _showWords = false;
    uint16_t _bump = 0;
    uint16_t _steps = 0;
    bool _verbose = false;

    int8_t _sex(uint8_t n, uint8_t b)
    {

        if ((n & (1 << b)) != 0)
        {
            uint8_t bits = (0xff >> b) << b;
            return n | bits;
        }

        return n;
    }

    void _printarg(uint8_t arg)
    {
        switch (arg)
        {
        case REG_0:
            printf("R0");
            break;
        case REG_1:
            printf("R1");
            break;
        case REG_2:
            printf("R2");
            break;
        case REG_3:
            printf("R3");
            break;
        case REG_4:
            printf("R4");
            break;
        case REG_5:
            printf("R5");
            break;
        case REG_6:
            printf("R6");
            break;
        case REG_7:
            printf("R7");
            break;
        case REG_A:
            printf("A");
            break;
        case REG_B:
            printf("B");
            break;
        case REG_CA:
            printf("CA");
            break;
        case REG_I:
            printf("I");
            break;
        case REG_PC:
            printf("PC");
            break;
        case REG_RS:
            printf("RS");
            break;
        case REG_SP:
            printf("SP");
            break;
        case REG_WA:
            printf("WA");
            break;
        default:
            break;
        }
    }
    void _printargi(uint8_t arg)
    {
        printf("(");
        _printarg(arg);
        printf(")");
    }
    void _printargs(uint8_t arga, uint8_t argb)
    {
        _printarg(arga);
        printf(",");
        _printarg(argb);
        printf("\n");
    }
};
#endif