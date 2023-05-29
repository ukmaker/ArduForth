
#include <stdio.h>
#include "RAM.h"
#include "ForthVM.h"
#include "TIL.h"
#include "Assembler.h"
#include "Test.h"

uint16_t memory[8192];

RAM ram(memory, 8192);

Syscall syscalls[40];

ForthVM vm(ram, syscalls, 40);

TIL til(vm);

Assembler fasm;

int tests = 0;
int passed = 0;
int failed = 0;

void printC(ForthVM *vm) {
    // Syscall to print the char on the top of the stack
    // ( c - )
    uint16_t v = vm->pop();
    char c = (char) v;

    printf("%c", c);
}

void shouldHalt() {

    ram.put(0, OP_HALT << OP_BITS);
    vm.reset();
    vm.step();
    assert(vm.halted(), "VM should be halted");

}

void shouldAdd() {
    vm.reset();
    vm.load(0,0,OP_LDAI, 3);
    vm.load(0,0,OP_LDBI, 7);
    vm.load(0,0,OP_ADD, REG_A, REG_B);
    vm.load(0,0,OP_HALT,0);
    vm.reset();
    vm.step();
    vm.step();
    vm.step();
    vm.step();
    assert(vm.get(REG_A) == 10, "Should add");    
}

void shouldGenerateCarry() {
    vm.reset();
    vm.load(0,0,OP_LDL, REG_A,0);
    vm.load(0x8000);
    vm.load(0,0,OP_LDL, REG_B,0);
    vm.load(0x8000);
    vm.load(0,0,OP_ADD, REG_A, REG_B);
    vm.load(0,0,OP_HALT,0);
    vm.reset();
    vm.step();
    vm.step();
    vm.step();
    vm.step();
    assert(vm.get(REG_A) == 0, "Should add");    
    assert(vm.getC(), "Should generate carry");    
}

void shouldSub() {
    vm.reset();
    vm.load(0,0,OP_LDAI, 3);
    vm.load(0,0,OP_LDBI, 7);
    vm.load(0,0,OP_SUB, REG_A, REG_B);
    vm.load(0,0,OP_HALT,0);
    vm.reset();
    vm.step();
    vm.step();
    vm.step();
    vm.step();
    assertEquals(vm.get(REG_A), -4, "SUB result"); 
    assert(vm.getC(), "SUB should set carry");   
}

void shouldSubL() {
    vm.reset();
    vm.load(0,0,OP_LDL, REG_A, 0);
    vm.load(0xaaaa);
    vm.load(0,0,OP_LDL, REG_B, 0);
    vm.load(0x1111);
    vm.load(0,0,OP_SUB, REG_A, REG_B);
    vm.load(0,0,OP_HALT,0);
    vm.reset();
    vm.step();
    vm.step();
    vm.step();
    vm.step();
    assertEquals(vm.get(REG_A), 0xaaaa - 0x1111, "SUBL result"); 
    assert(!vm.getC(), "SUB should not set carry");   
}

void shouldMulL() {
    vm.reset();
    vm.load(0,0,OP_LDL, REG_A, 0);
    vm.load(0x1111);
    vm.load(0,0,OP_LDL, REG_B, 0);
    vm.load(4);
    vm.load(0,0,OP_MUL, REG_A, REG_B);
    vm.load(0,0,OP_HALT,0);
    vm.reset();
    vm.step();
    vm.step();
    vm.step();
    vm.step();
    assertEquals(vm.get(REG_A), 0x4444, "MUL result"); 
    assert(!vm.getC(), "MUL should not set carry");   
}

void shouldAnd() {
    vm.reset();
    vm.load(0,0,OP_LDL, REG_A, 0);
    vm.load(0x1111);
    vm.load(0,0,OP_LDL, REG_B, 0);
    vm.load(0x1010);
    vm.load(0,0,OP_AND, REG_A, REG_B);
    vm.load(0,0,OP_HALT,0);
    vm.reset();
    vm.step();
    vm.step();
    vm.step();
    vm.step();
    assertEquals(vm.get(REG_A), 0x1010, "AND result"); 
    assert(!vm.getC(), "AND should not set carry");   
}

void shouldPrintC() {
    syscalls[0] = printC;
    vm.reset();
    vm.load(0,0,OP_LDL, REG_SP, 0);
    vm.load((uint16_t)0);
    vm.load(0,0,OP_LDL, REG_A, 0);
    vm.load('F');
    vm.load(0,0,OP_PUSHD, REG_A, 0);
    vm.load(0,0,OP_SYSCALL, 0);
    vm.load(0,0,OP_HALT,0);
    vm.reset();
    vm.step();
    vm.step();
    vm.step();
    vm.step(); 
   vm.step(); 

    assertEquals(vm.get(REG_SP), 0, "SP should be zero after printC");
}

void shouldLDBI() {
    vm.reset();
    vm.load(0,0,OP_LDBI, 5);
    vm.reset();
    vm.step();

    assertEquals(vm.get(REG_B), 5, "B should be 5");
}

void shouldAddI() {
    vm.reset();
    vm.load(0,0,OP_LDBI, 5);
    vm.load(0,0,OP_ADDI, REG_B, -2);
    vm.reset();
    vm.step();
    vm.step();

    assertEquals(vm.get(REG_B), 3, "B should be 3 after add");
}

void shouldOpenAsmFile() {
    printf("         shouldOpenAsmFile\n");
    assert(fasm.slurp("tests/test-slurp.asm"), "Failed to open file test-slurp.asm");
    assertEquals(fasm.fileSize(), 880, "Wrong file size");
}

void shouldGetOpeningComment() {
    printf("         shouldGetOpeningComment\n");
    Token *tok = fasm.getToken();
    assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize a standalone comment");
    assertEquals(tok->type, TOKEN_TYPE_COMMENT, "Type should be COMMENT");
    assertEquals(tok->line, 1, "First comment is on line 1");
    assertEquals(tok->pos, 2, "First comment is on pos 2");
}

void shouldGetSecondComment() {
    printf("         shouldGetSecondComment\n");
    Token *tok = fasm.getToken();
    assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize a standalone comment");
    assertEquals(tok->type, TOKEN_TYPE_COMMENT, "Type should be COMMENT");
    assertEquals(tok->line, 2, "Second comment is on line 2");
    assertEquals(tok->pos, 1, "Second comment is on pos 1");
}

void shouldGetHexConstant() {
    printf("         shouldGetHexConstant\n");
    Token *tok = fasm.getToken();
    assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize a HEX constant");
    assertEquals(tok->type, TOKEN_TYPE_CONST, "Type should be CONST");
    assertEquals(tok->line, 5, "Constant is on line 5");
    assertEquals(tok->pos, 1, "Constant is on pos 1");
    assertEquals(strcmp(tok->name, "#HNAME"), 0, "Should parse CONST label");
    assertEquals(tok->value, 0xFA90, "Should parse a hex value");
    // and we should parse the comment at the end of the line
    tok = fasm.getToken();
    assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize an end of line comment");
    assertEquals(tok->type, TOKEN_TYPE_COMMENT, "Type should be COMMENT");
}
void shouldGetDecConstant() {
    printf("         shouldGetDecConstant\n");
    Token *tok = fasm.getToken();
    assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize a DEC constant");
    assertEquals(tok->type, TOKEN_TYPE_CONST, "Type should be CONST");
    assertEquals(tok->line, 6, "Constant is on line 6");
    assertEquals(tok->pos, 1, "Constant is on pos 1");
    assertEquals(strcmp(tok->name, "#DNAME"), 0, "Should parse CONST label");
    assertEquals(tok->value, 3900, "Should parse a decimal value");
    // and we should parse the comment at the end of the line
    tok = fasm.getToken();
    assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize an end of line comment");
    assertEquals(tok->type, TOKEN_TYPE_COMMENT, "Type should be COMMENT");
}
void shouldGetBinConstant() {
    printf("         shouldGetBinConstant\n");
    Token *tok = fasm.getToken();
    assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize a BIN constant");
    assertEquals(tok->type, TOKEN_TYPE_CONST, "Type should be CONST");
    assertEquals(tok->line, 7, "Constant is on line 7");
    assertEquals(tok->pos, 1, "Constant is on pos 1");
    assertEquals(strcmp(tok->name, "#BNAME"), 0, "Should parse CONST label");
    assertEquals(tok->value, 0b10010110, "Should parse a binary value");
    // and we should parse the comment at the end of the line
    tok = fasm.getToken();
    assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize an end of line comment");
    assertEquals(tok->type, TOKEN_TYPE_COMMENT, "Type should be COMMENT");
}

void shouldGetVariable() {
    printf("         shouldGetVariable\n");
    Token *tok = fasm.getToken();
    assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize a variable");
    assertEquals(tok->type, TOKEN_TYPE_VAR, "Type should be VAR");
    assertEquals(tok->line, 10, "Var is on line 10");
    assertEquals(tok->pos, 1, "Var is on pos 1");
    assertString(tok->name, "%VNAME", "Should parse VAR label");
    // and we should parse the comment at the end of the line
    tok = fasm.getToken();
    assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize an end of line comment");
    assertEquals(tok->type, TOKEN_TYPE_COMMENT, "Type should be COMMENT");
}

void shouldGetString() {
    printf("         shouldGetString\n");
    Token *tok = fasm.getToken();
    assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize a string label");
    assertEquals(tok->type, TOKEN_TYPE_STR, "Type should be STRING");
    assertEquals(tok->line, 11, "String is on line 11");
    assertEquals(tok->pos, 1, "String is on pos 1");
    assertString(tok->name, "$SNAME", "Should parse String label");
    assertString(tok->str, "This is a string", "Should have the correct string value");
    // and we should parse the comment at the end of the line
    tok = fasm.getToken();
    assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize an end of line comment");
    assertEquals(tok->type, TOKEN_TYPE_COMMENT, "Type should be COMMENT");
}


void shouldGetOrg1() {
    printf("         shouldGetOrg1\n");
    Token *tok = fasm.getToken();
    assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize a line label");
    assertEquals(tok->type, TOKEN_TYPE_DIRECTIVE, "Type should be DIRECTIVE");
    assertEquals(tok->line, 9, "ORG is on line 9");
    assertEquals(tok->pos, 1, "ORG is on pos 1");
    assertEquals(tok->value, 1000, "Should parse value");
    // and we should parse the comment at the end of the line
    tok = fasm.getToken();
    assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize an end of line comment");
    assertEquals(tok->type, TOKEN_TYPE_COMMENT, "Type should be COMMENT");
}

void shouldGetOrg2() {
    printf("         shouldGetOrg2\n");
    Token *tok = fasm.getToken();
    assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize a line label");
    assertEquals(tok->type, TOKEN_TYPE_DIRECTIVE, "Type should be DIRECTIVE");
    assertEquals(tok->line, 13, "ORG is on line 13");
    assertEquals(tok->pos, 1, "ORG is on pos 1");
    assertEquals(tok->value, 0, "Should parse value");
    // and we should parse the comment at the end of the line
    tok = fasm.getToken();
    assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize an end of line comment");
    assertEquals(tok->type, TOKEN_TYPE_COMMENT, "Type should be COMMENT");
}

void shouldGetLabel() {
    printf("         shouldGetLabel\n");
    Token *tok = fasm.getToken();
    assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize a line label");
    assertEquals(tok->type, TOKEN_TYPE_LABEL, "Type should be LABEL");
    assertEquals(tok->line, 14, "Label is on line 14");
    assertEquals(tok->pos, 1, "label is on pos 1");
    assertString(tok->name, "START", "Should parse label");
}

void shouldGetLD() {
    printf("         shouldGetLD\n");
    Token *tok = fasm.getToken();
    assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize an opcode");
    assertEquals(tok->type, TOKEN_TYPE_OPCODE, "Type should be OPCODE");
    assertEquals(tok->opcode, OP_LD, "Should be a LD instruction");
    // and we should parse the comment at the end of the line
    tok = fasm.getToken();
    assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize an end of line comment");
    assertEquals(tok->type, TOKEN_TYPE_COMMENT, "Type should be COMMENT");
}

void shouldGetNOP() {
    printf("         shouldGetNOP\n");
    Token *tok = fasm.getToken();
    assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize an opcode");
    assertEquals(tok->type, TOKEN_TYPE_OPCODE, "Type should be OPCODE");
    assertEquals(tok->opcode, OP_NOP, "Should be a NOP instruction");
    // and we should parse the comment at the end of the line
    tok = fasm.getToken();
    assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize an end of line comment");
    assertEquals(tok->type, TOKEN_TYPE_COMMENT, "Type should be COMMENT");
}

int main(int argc, char **argv) {
    
    shouldHalt();
    shouldAdd();
    shouldGenerateCarry();
    shouldSub();
    shouldSubL();
    shouldMulL();
    shouldAnd();

    shouldPrintC();

    shouldLDBI();
    shouldAddI();

    shouldOpenAsmFile();

    shouldGetOpeningComment();
    shouldGetSecondComment();

    shouldGetHexConstant();
    shouldGetDecConstant();
    shouldGetBinConstant();

    shouldGetOrg1();
    shouldGetVariable();
    shouldGetString();


    shouldGetOrg2();
    shouldGetLabel();

    shouldGetLD();
    shouldGetNOP();

    printf("==============================\n");
    printf("TOTAL: %d  PASSED %d  FAILED %d\n", Test::tests, Test::passed, Test::failed);
    printf("==============================\n");
    
    fasm.pass1();
   fasm.pass2();
   fasm.pass3();

    fasm.dump();
    fasm.writeCode();

    /*

    vm.setDebugging(true);
    til.boot();
    til.run();

    */

    return 0;
}