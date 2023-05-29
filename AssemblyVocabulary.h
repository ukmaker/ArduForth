#ifndef UKMAKE_ASSEMBLY_VOCABULARY_H
#define UKMAKE_ASSEMBLY_VOCABULARY_H

#include <string.h>
#include <stdint.h>
#include "ForthIS.h"

class AssemblyVocabulary {

    public:
    AssemblyVocabulary() {

        for(int i=0; i<64; i++) opnames[i] = NULL;


        /**
         * No arguments
         */ 
        opnames[OP_RET] = "RET";
        opnames[OP_HALT] = "HALT";
        opnames[OP_NOP] = "NOP";

        /**
         *  Two arguments
         */
        opnames[OP_LD] = "LD"; // Ra <- Rb  
        opnames[OP_ADD] = "ADD";  // Ra <- Ra + Rb
        opnames[OP_SUB] = "SUB"; // Ra <- Ra - Rb
        opnames[OP_MUL] = "MUL"; // Ra <- Ra * Rb
        opnames[OP_DIV] = "DIV"; // Ra <- Ra / Rb
        opnames[OP_AND] = "AND"; // Ra <- Ra and Rb
        opnames[OP_OR] = "OR"; // Ra <- Ra or Rb
        opnames[OP_XOR] = "XOR"; // Ra <- Ra xor Rb
        opnames[OP_CMP] = "CMP"; // CMP Ra,Rb

        // Written STAIX SP,I to mean (SP + I) <- A
        opnames[OP_STAIX] = "STAIX";   // (Ra + Rb) <- RA
        opnames[OP_STBIX] = "STBIX";   // (Ra + Rb) <- RB
        // Written STAIXB WC,I to mean (WA + I) <- AL
        opnames[OP_STAIXB] = "STAIXB";   // (Ra + Rb) <- RA
        opnames[OP_STBIXB] = "STBIXB";   // (Ra + Rb) <- RB
        opnames[OP_FETCH] = "FETCH"; // Ra <- (Rb)
        opnames[OP_ST ] = "ST";   // (Ra) <- Rb

        /**
         * register and 3-bit tiny immediates
         */
        opnames[OP_LDI] = "LDI";  // Ra <- #num3 - load tiny numbers
        opnames[OP_STI] = "STI";   // (Ra) <- #num3
        opnames[OP_ADDI] = "ADDI";  // Ra <- Ra + #num3 (signed)
        opnames[OP_CMPI] = "CMPI"; // CMP Ra,#num3

        /**
         *  6-bit immediates
         */
        opnames[OP_LDAI] = "LDAI";   // RA <- #num6 - load medium numbers
        opnames[OP_LDBI] = "LDBI";   // RB <- #num6 - load medium numbers
        opnames[OP_STAI] = "STAI";   // (RA) <- #num6 stores the sign-extended word
        opnames[OP_STBI] = "STBI";   // (RB) <- #num6
        // Byte-oriented - stores the low-order byte at the calculated byte-address
        opnames[OP_STAIB] = "STAIB"; // (RA) <- #num6 where (RA) is the byte address
        opnames[OP_STBIB] = "STBIB"; // (RA) <- #num6
        opnames[OP_ADDAI] = "ADDAI"; //RA <- RA + #num6 (signed)
        opnames[OP_ADDBI] = "ADDBI"; //RB <- RB + #num6 (signed)
        opnames[OP_CMPAI] = "CMPAI"; // CMP RA,#num6
        opnames[OP_CMPBI] = "CMPBI"; // CMP RB,#num6
        opnames[OP_JR] = "JR";       // PC <- PC + #num6 << 1
        opnames[OP_CALLR] = "CALLR"; // PC <- PC + #num6 << 1
        opnames[OP_SYSCALL] = "SYSCALL"; // call a high-level routine <call.6>
        
        // arga and long immediates
        opnames[OP_LDL]  = "LDL";   // Ra <- #num16
        opnames[OP_STL]  = "STL";   // (Ra) <- #num16
        opnames[OP_ADDL]  = "ADDL"; // Ra <- Ra + #num16
        opnames[OP_CMPL]  = "CMPL"; // CMP Ra,#num16
        opnames[OP_JPIDX] = "JPIDX"; // PC <- Ra + #num16

        // long immediates, arga and argb ignored
        opnames[OP_JP]    = "JP";   // PC <- #num16
        opnames[OP_CALL]  = "CALL"; // PC <- #num16

        // single register arguments (in arga slot, argb slot is ignored)
        opnames[OP_PUSHD] = "PUSHD"; // (SP) <- Ra, INC SP
        opnames[OP_PUSHR] = "PUSHR"; // (RS) <- Ra, INC RS
        opnames[OP_POPD] = "POPD";   // DEC SP, Ra <- (SP)
        opnames[OP_POPR] = "POPR";   // DEC RS, Ra <- (RS)
        opnames[OP_NOT] = "NOT";     // Ra <- !Ra
        opnames[OP_SL] = "SL";       // Ra <- Ra << 1
        opnames[OP_SR] = "SR";       // Ra <- Ra >> 1

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

        directives[0] = "ORG";

    }

    const char *opname(uint8_t opcode) {
        if(opcode > 63) return NULL;
        return opnames[opcode];
    }
   
    const char *argname(uint8_t arg) {
        if(arg > 7) return NULL;
        return argnames[arg];
    }
   
    const char *ccname(uint8_t cc) {
        if(cc > 63) return NULL;
        return ccnames[cc];
    }

    const char *directive(uint8_t dir) {
        if(dir > 1) return NULL;
        return directives[dir];
    }

    int findOpcode(char *source, int pos, int len) {
        for(int i=0; i<64; i++) {
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

    int findArg(char *source, int pos, int len) {
        for(int i=0; i<8; i++) {
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
        return -1;
    }

    int findDirective(char *source, int pos, int len) {
        for(int i=0; i<1; i++) {
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

        const char *opnames[64];
        const char *argnames[8];
        const char *ccnames[4];
        const char *directives[1];
};
#endif