#ifndef UKMAKER_ASSEMBLY_VOCABULARY_H
#define UKMAKER_ASSEMBLY_VOCABULARY_H

#include <string.h>
#include <stdint.h>
#include "../runtime/ArduForth/ForthIS.h"
#include "Token.h"

class AssemblyVocabulary {

    public:
    AssemblyVocabulary() {

        for(int i=0; i<256; i++) opnames[i] = NULL;
        for(int i=0; i<65; i++) {
            for(int j=0; j<16; j++) {
                aliases[j][i] = '\0';
            }
        }

        opnames[OP_NOP] = "NOP";

        opnames[OP_MOV] = "MOV";
        opnames[OP_MOVI] = "MOVI";
        opnames[OP_MOVIL] = "MOVIL";
        opnames[OP_MOVAI] = "MOVAI";
        opnames[OP_MOVBI] = "MOVBI";

        opnames[OP_LD] = "LD";
        opnames[OP_LD_B] = "LD_B";
        opnames[OP_LDAX] = "LDAX";
        opnames[OP_LDBX] = "LDBX";
        opnames[OP_LDAX_B] = "LDAX_b";
        opnames[OP_LDBX_B] = "LDBX_B";

        opnames[OP_STI] = "STI";
        opnames[OP_STAI] = "STAI";
        opnames[OP_STBI] = "STBI";
        opnames[OP_STIL] = "STIL";
        opnames[OP_STI_B] = "STI_B";
        opnames[OP_STAI_B] = "STAI_B";
        opnames[OP_STBI_B] = "STBI_B";
                
        opnames[OP_ST] = "ST";
        opnames[OP_ST_B] = "ST_B";

        opnames[OP_STXA] = "STXA";
        opnames[OP_STXB] = "STXB";
        opnames[OP_STXA_B] = "STXA_B";
        opnames[OP_STXB_B] = "STXB_B";

        opnames[OP_PUSHD] = "PUSHD";
        opnames[OP_PUSHR] = "PUSHR";
        opnames[OP_POPD] = "POPD";
        opnames[OP_POPR] = "POPR";

        opnames[OP_ADD] = "ADD";
        opnames[OP_ADDI] = "ADDI";
        opnames[OP_ADDAI] = "ADDAI";
        opnames[OP_ADDBI] = "ADDBI";
        opnames[OP_ADDIL] = "ADDIL";

        opnames[OP_SUB] = "SUB";
        opnames[OP_SUBI] = "SUBI";
        opnames[OP_SUBAI] = "SUBAI";
        opnames[OP_SUBBI] = "SUBBI";
        opnames[OP_SUBIL] = "SUBIL";

        opnames[OP_MUL] = "MUL";
        opnames[OP_DIV] = "DIV";

        opnames[OP_AND] = "AND";
        opnames[OP_OR] = "OR";
        opnames[OP_XOR] = "XOR";
        opnames[OP_NOT] = "NOT";

        opnames[OP_SL] = "SL";
        opnames[OP_SR] = "SR";
        opnames[OP_RL] = "RL";
        opnames[OP_RLC] = "RLC";
        opnames[OP_RR] = "RR";
        opnames[OP_RRC] = "RRC";

        opnames[OP_BIT] = "BIT";
        opnames[OP_SET] = "SET";
        opnames[OP_CLR] = "CLR";

        opnames[OP_SLI] = "SLI";
        opnames[OP_SRI] = "SRI";
        opnames[OP_RLI] = "RLI";
        opnames[OP_RLCI] = "RLCI";
        opnames[OP_RRI] = "RRI";
        opnames[OP_RRCI] = "RRCI";

        opnames[OP_BITI] = "BITI";
        opnames[OP_SETI] = "SETI";
        opnames[OP_CLRI] = "CLRI";

        opnames[OP_CMP] = "CMP";
        opnames[OP_CMPI] = "CMPI";
        opnames[OP_CMPAI] = "CMPAI";
        opnames[OP_CMPBI] = "CMPBI";

        opnames[OP_RET] = "RET";
        opnames[OP_SYSCALL] = "SYSCALL";
        opnames[OP_HALT] = "HALT";
        opnames[OP_BRK] = "BRK";

        opnames[OP_JP] = "JP";
        opnames[OP_JR] = "JR";
        opnames[OP_JX] = "JX";
        opnames[OP_JXL] = "JXL";

        opnames[OP_CALL] = "CALL";
        opnames[OP_CALLR] = "CALLR";
        opnames[OP_CALLX] = "CALLX";
        opnames[OP_CALLXL] = "CALLXL";


        argnames[REG_0] = "R0";
        argnames[REG_1] = "R1";
        argnames[REG_2] = "R2";
        argnames[REG_3] = "R3";
        argnames[REG_4] = "R4";
        argnames[REG_5] = "R5";
        argnames[REG_6] = "R6";
        argnames[REG_7] = "R7";
        argnames[REG_A] = "A";
        argnames[REG_B] = "B";
        argnames[REG_PC] = "PC";
        argnames[REG_WA] = "WA";
        argnames[REG_SP] = "SP";
        argnames[REG_RS] = "RS";
        argnames[REG_CA] = "CA";
        argnames[REG_I] = "I";

        ccnames[COND_C] = "C";
        ccnames[COND_Z] = "Z";
        ccnames[COND_P] = "P";
        ccnames[COND_M] = "M";

        directives[DIRECTIVE_TYPE_ORG] = "ORG";
        directives[DIRECTIVE_TYPE_DATA] = "DATA";
        directives[DIRECTIVE_TYPE_ALIAS] = "ALIAS";
        directives[DIRECTIVE_TYPE_PLAIN_STRING] = "SDATA";
        directives[DIRECTIVE_TYPE_NWORD_STRING] = "N";
        directives[DIRECTIVE_TYPE_RWORD_STRING] = "R";
        directives[DIRECTIVE_TYPE_IWORD_STRING] = "I";
        directives[DIRECTIVE_TYPE_XWORD_STRING] = "X";

    }

    void setAlias(uint8_t argname, const char *alias) {
        strcpy(aliases[argname],alias);
    }

    const char *opname(uint8_t opcode) {
        if(opcode > 255) return NULL;
        return opnames[opcode];
    }
   
    const char *argname(uint8_t arg) {
        if(arg > 15) return NULL;
        return argnames[arg];
    }
   
    const char *ccname(uint8_t cc) {
        if(cc > 3) return NULL;
        return ccnames[cc];
    }

    const char *directive(uint8_t dir) {
        if(dir > 7) return NULL;
        return directives[dir];
    }

    int findOpcode(char *source, int pos, int len) {
        for(int i=0; i<256; i++) {
            if(opnames[i] != NULL && (int)strlen(opnames[i]) == len) {
                int equal = true;
                for(int j=0; j<len; j++) {
                    if(source[pos + j] != opnames[i][j]) {
                        equal = false;
                    }
                }
                if(equal) return i;
            }
        }
        return -1;
    }

    int findAlias(char *source, int pos, int len) {
        for(int i=0; i<16; i++) {
            if((int)strlen(aliases[i]) == len) {
                int equal = true;
                for(int j=0; j<len; j++) {
                    if(source[pos + j] != aliases[i][j]) {
                        equal = false;
                    }
                }
                if(equal) return i;
            }
        }
        return -1;
    }

    int findArg(char *source, int pos, int len) {
        for(int i=0; i<16; i++) {
            if((int)strlen(argnames[i]) == len) {
                int equal = true;
                for(int j=0; j<len; j++) {
                    if(source[pos + j] != argnames[i][j]) {
                        equal = false;
                    }
                }
                if(equal) return i;
            }
        }
        return findAlias(source, pos, len);
    }

    int findDirective(char *source, int pos, int len) {
        for(int i=0; i<8; i++) {
            if((int)strlen(directives[i]) == len) {
                int equal = true;
                for(int j=0; j<len; j++) {
                    if(source[pos + j] != directives[i][j]) {
                        equal = false;
                    }
                }
                if(equal) return i;
            }
        }
        return -1;
    }

    protected:

        const char *opnames[256];
        const char *argnames[16];
        const char *ccnames[4];
        const char *directives[8];
        char aliases[16][65];
};
#endif