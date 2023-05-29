#ifndef UKMAKER_FORTH_IS_H
#define UKMAKER_FORTH_IS_H


#define REG_A 0
#define REG_B 1
#define REG_PC 2
#define REG_WA 3
#define REG_SP 4
#define REG_RS 5
#define REG_CA 6
#define REG_I 7

// Opcodes
// <capply.2><opcode.6> <cond.2><a.3><b.3>

// <capply.2><opcode.6> <cond.2><a.3><num.3>

// <capply.2><opcode.6> <cond.2><num.6>

// <capply.2><opcode.6> <cond.2><xx.6>
// <num.16>


#define CC_APPLY_BITS 14 // Shift up this many bits
#define CC_APPLY_MASK (0x03 << CC_APPLY_BITS)
#define COND_APPLY_OFF 0
#define COND_APPLY_MATCH 2
#define COND_APPLY_NOT_MATCH 3
#define COND_APPLY_IGN 1

#define CC_BITS 6 // Shift up this many bits
#define CC_MASK (0x03 << CC_BITS)

#define COND_C 0 // Carry
#define COND_Z 1 // Zero
#define COND_M 2 // Negative
#define COND_P 3 // Even parity

#define OP_BITS 8
#define OP_MASK (0x3f << OP_BITS)

#define OP_NOP 0

#define OP_LD 1    // Ra <- Rb
#define OP_LDI 2   // Ra <- #num3 - load tiny numbers
#define OP_LDAI 3   // RA <- #num6 - load medium numbers
#define OP_LDBI 4   // RB <- #num6 - load medium numbers
#define OP_LDL 5   // Ra <- #num16
#define OP_FETCH 6 // Ra <- (Rb)
#define OP_ST 7    // (Ra) <- Rb
#define OP_STI 8   // (Ra) <- #num3
#define OP_STAI 9   // (RA) <- #num6
#define OP_STBI 10   // (RB) <- #num6
#define OP_STAIX 11   // (Ra + Rb) <- RA
#define OP_STBIX 12   // (Ra + Rb) <- RB
#define OP_STL 13   // (Ra) <- #num16
#define OP_PUSHD 14 // DSTACK <- Ra, INC SP
#define OP_PUSHR 15
#define OP_POPD 16 // Ra <- DSTACK, DEC SP
#define OP_POPR 17

#define OP_ADD 18
#define OP_ADDI 19 // Ra <- Ra + #num3 (signed)
#define OP_ADDAI 20 //RA <- RA + #num6 (signed)
#define OP_ADDBI 21 //RB <- RB + #num6 (signed)
#define OP_ADDL 22 // Ra <- Ra + #num16

#define OP_SUB 23
#define OP_MUL 24
#define OP_DIV 25
#define OP_AND 26
#define OP_OR 27
#define OP_NOT 28
#define OP_XOR 29
#define OP_SL 30
#define OP_SR 31

#define OP_CMP 32 // CMP Ra,Rb
#define OP_CMPI 33 // CMP Ra,#num3
#define OP_CMPAI 34 // CMP RA,#num6
#define OP_CMPBI 35 // CMP RB,#num6
#define OP_CMPL 36 // CMP Ra,#num16

#define OP_JR 37  // PC <- PC + #num6.0 [ equivalent to OP_LDI PC,#num7 ]
#define OP_JP 38 // PC <- #num16
#define OP_JPIDX 39 // PC <- Ra + #num16

#define OP_CALL 40 // PC <- #num16
#define OP_CALLR 41 // PC <- PC + #num6.0
#define OP_RET 42
#define OP_SYSCALL 43 // call a high-level routine <call.6>
#define OP_HALT 44

// Byte-oriented - stores the low-order byte at the calculated byte-address
#define OP_STAIXB 45  // (Ra + Rb) <- RAl
#define OP_STBIXB 46   // (Ra + Rb) <- RBl
#define OP_STAIB 47  // (Ra + Rb) <- RAl
#define OP_STBIB 48   // (Ra + Rb) <- RBl

#define ARGA_BITS 3
#define ARGB_BITS 0
#define ARGA_MASK (0x07 << ARGA_BITS)
#define ARGB_MASK (0x07 << ARGB_BITS)


#endif