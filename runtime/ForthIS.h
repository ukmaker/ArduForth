#ifndef UKMAKER_FORTH_IS_H
#define UKMAKER_FORTH_IS_H

// General purpose registers
#define REG_0 0
#define REG_1 1
#define REG_2 2
#define REG_3 3
#define REG_4 4
#define REG_5 5
#define REG_6 6
#define REG_7 7

// Special purpose registers
// quick registers
#define REG_A 8
#define REG_B 9
// Forth instruction pointer
#define REG_I 10
// Forth code address pointer
#define REG_CA 11
// Forth word address pointer
#define REG_WA 12
// Data stack pointer
#define REG_SP 13
// Return stack pointer
#define REG_RS 14
// Program counter
#define REG_PC 15

// Opcodes

#define OP_BITS 8
#define OP_MASK (0x7f << OP_BITS)

#define OP_NOP 0

#define OP_MOV 1    // Ra <- Rb
#define OP_MOVI 2   // Ra <- #num4 - load small numbers
#define OP_MOVIL 3   // Ra <- #num16
#define OP_MOVAI 4   // RA <- #num8 - load medium numbers
#define OP_MOVBI 5   // RB <- #num8 - load medium numbers

#define OP_LD 6      // Ra <- (Rb)
#define OP_LD_B 7     // Ra <- (Rb)l
#define OP_LDAX 8    // RA <- (Ra + #n4.0) - word address, sign extended
#define OP_LDBX 9    // RB <- (Ra + #n4.0)
#define OP_LDAX_B 10  // RA <- (Ra + #n4) - byte address, sign extended
#define OP_LDBX_B 11  // RB <- (Ra + #n4) - byte address, sign extended

#define OP_STI 12    // (Ra) <- #num4
#define OP_STAI 13   // (RA) <- #num8
#define OP_STBI 14   // (RB) <- #num8
#define OP_STIL 15   // (Ra) <- #num16
#define OP_STI_B 18    // (Ra) <- #num4 l - byte address, sign extended
#define OP_STAI_B 19   // (RA) <- #num8 l - byte address, sign extended
#define OP_STBI_B 20   // (RB) <- #num8 l - byte address, sign extended

#define OP_ST 16     // (Ra) <- Rb
#define OP_ST_B 17   // (Ra) <- Rbl - byte address 

#define OP_STXA 21   // (Ra + #n4.0) <- RA
#define OP_STXB 22   // (Ra + #n4.0) <- RB
#define OP_STXA_B 23  // (Ra + #n4) <- RA - byte address, sign extended
#define OP_STXB_B 24   // (Rb + #n4) <- RB - byte address, sign extended


#define OP_PUSHD 25 // DSTACK <- Ra, INC SP
#define OP_PUSHR 26
#define OP_POPD 27 // Ra <- DSTACK, DEC SP
#define OP_POPR 28

#define OP_ADD 29
#define OP_ADDI 30 // Ra <- Ra + #num4 (signed)
#define OP_ADDAI 31 //RA <- RA + #num8 (signed)
#define OP_ADDBI 32 //RB <- RB + #num8 (signed)
#define OP_ADDIL 33 // Ra <- Ra + #num16

#define OP_SUB 34  // Ra <- Ra - Rb
#define OP_SUBI 35 // Ra <- #num4 - Ra
#define OP_SUBAI 36 // RA <- #num8 - RA
#define OP_SUBBI 37
#define OP_SUBIL 38 // Ra <- #num16 - Ra

#define OP_MUL 39
#define OP_DIV 40
#define OP_AND 41
#define OP_OR 42
#define OP_NOT 43
#define OP_XOR 44

#define OP_SL 45 // Ra <- Ra << Rb
#define OP_SR 46 // Ra <- Ra >> Rb
#define OP_RR 47 // Ra <- Ra << Rb
#define OP_RRC 48  // Ra <- Ra << Rb
#define OP_RL 49 // Ra <- Ra << Rb
#define OP_RLC 50 // Ra <- Ra << Rb

#define OP_BIT 51 // Z <- Ra & bit(Rb)
#define OP_SET 52 // Ra <- Ra or bit(Rb)
#define OP_CLR 53 // Ra <- Ra & !bit(Rb)

#define OP_SLI 54 // Ra <- Ra << #u4
#define OP_SRI 55 // Ra <- Ra >> #u4
#define OP_RRI 56 // Ra <- Ra << #u4
#define OP_RRCI 57  // Ra <- Ra << #u4
#define OP_RLI 58 // Ra <- Ra << #u4
#define OP_RLCI 59 // Ra <- Ra << #u4

#define OP_BITI 60 // Z <- Ra & bit(#n4)
#define OP_SETI 61 // Ra <- Ra or bit(#n4)
#define OP_CLRI 62 // Ra <- Ra & !bit(#n4)

#define OP_CMP 63 // CMP Ra,Rb
#define OP_CMPI 64 // CMP Ra,#num4
#define OP_CMPAI 65 // CMP RA,#num8 - sign exended
#define OP_CMPBI 66 // CMP RB,#num8 - sign extended
#define OP_CMPIL 67 // CMP Ra,#num16

#define OP_RET 68
#define OP_SYSCALL 69 // call a high-level routine <call.6>
#define OP_HALT 70
#define OP_BRK 71

// Jumps and calls
// All can have conditions applied
// Written e.g JR[NZ] #17
//
// Bits are
// 15    - 0: general instructions; 1 - jump or call instructions
// 14    - 0: jumps;                1 - calls
// 13    - 0: unconditional;        1 - conditional
// 12    - 0: condition normal;     1 - condition inverted
// 11,10 - Condition codes 00:C, 01:Z, 10:P, 11:M
// 9,8   - 00: immediate; 01: relative; 10: indexed short; 11: indexed long
// 
#define OP_JP 0x80    // PC <- #num16
#define OP_JR 0x81    // PC <- PC + #num8.0
#define OP_JX 0x82    // PC <- Ra + #num4.0
#define OP_JXL 0x83   // PC <- Ra + #num16

#define OP_CALL 0xc0   // PC <- #num16
#define OP_CALLR 0xc1  // PC <- PC + #num8.0
#define OP_CALLX 0xc2  // PC <- Ra + #num4.0
#define OP_CALLXL 0xc3 // PC <- Ra + #num16

#define COND_C 0
#define COND_Z 1
#define COND_P 2
#define COND_M 3

#define CC_BITS 10
#define CC_MASK (0x03 << CC_BITS)
#define CC_INV_BIT 12
#define CC_INV_MASK (0x01 << CC_INV_BIT)
#define CC_APPLY_BIT 13
#define CC_APPLY_MASK (0x01 << CC_APPLY_BIT)
#define CC_ALL_BITS 10
#define CC_ALL_MASK (0x0f << CC_ALL_BITS)
#define JP_OR_CALL_BIT 15
#define JP_OR_CALL_MASK (0x01 << JP_OR_CALL_BIT)
#define JP_OR_CALL_OP_MASK ~(CC_ALL_MASK)


#define ARGA_BITS 4
#define ARGB_BITS 0
#define ARGA_MASK (0x0f << ARGA_BITS)
#define ARGB_MASK (0x0f << ARGB_BITS)


#endif