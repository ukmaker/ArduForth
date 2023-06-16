#ifndef UKMAKER_VMTEST_H
#define UKMAKER_VMTESTS_H

#include "Test.h"

void VMTests_printC(ForthVM *vm) {
    // Syscall to print the char on the top of the stack
    // ( c - )
    uint16_t v = vm->pop();
    char c = (char) v;

    printf("%c", c);
}

class VMTests : public Test {

    public:
    VMTests(TestSuite *suite, ForthVM *fvm, Assembler *vmasm) : Test(suite, fvm, vmasm) {}

    void run() {

    testSuite->reset();

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
    printf("==============================\n");
    printf("TOTAL: %d  PASSED %d  FAILED %d\n", testSuite->tests, testSuite->passed, testSuite->failed);
    printf("==============================\n");
}


void shouldHalt() {

    vm->ram()->put(0, OP_HALT << OP_BITS);
    vm->reset();
    vm->step();
    assert(vm->halted(), "VM should be halted");

}

void shouldAdd() {
    vm->reset();
    vm->load(0,0,OP_MOVAI, 3);
    vm->load(0,0,OP_MOVBI, 7);
    vm->load(0,0,OP_ADD, REG_A, REG_B);
    vm->load(0,0,OP_HALT,0);
    vm->reset();
    vm->step();
    vm->step();
    vm->step();
    vm->step();
    assert(vm->get(REG_A) == 10, "Should add");    
}

void shouldGenerateCarry() {
    vm->reset();
    vm->load(0,0,OP_MOVIL, REG_0,0);
    vm->load(0x8000);
    vm->load(0,0,OP_MOVIL, REG_1,0);
    vm->load(0x8000);
    vm->load(0,0,OP_ADD, REG_0, REG_1);
    vm->load(0,0,OP_HALT,0);
    vm->reset();
    vm->step();
    vm->step();
    vm->step();
    vm->step();
    assert(vm->get(REG_0) == 0, "Should add");    
    assert(vm->getC(), "Should generate carry");    
}

void shouldSub() {
    vm->reset();
    vm->load(0,0,OP_MOVAI, 3);
    vm->load(0,0,OP_MOVBI, 7);
    vm->load(0,0,OP_SUB, REG_A, REG_B);
    vm->load(0,0,OP_HALT,0);
    vm->reset();
    vm->step();
    vm->step();
    vm->step();
    vm->step();
    assertEquals(vm->get(REG_A), -4, "SUB result"); 
    assert(vm->getC(), "SUB should set carry");   
}

void shouldSubL() {
    vm->reset();
    vm->load(0,0,OP_MOVIL, REG_A, 0);
    vm->load(0xaaaa);
    vm->load(0,0,OP_MOVIL, REG_B, 0);
    vm->load(0x1111);
    vm->load(0,0,OP_SUB, REG_A, REG_B);
    vm->load(0,0,OP_HALT,0);
    vm->reset();
    vm->step();
    vm->step();
    vm->step();
    vm->step();
    assertEquals(vm->get(REG_A), 0xaaaa - 0x1111, "SUBL result"); 
    assert(!vm->getC(), "SUB should not set carry");   
}

void shouldMulL() {
    vm->reset();
    vm->load(0,0,OP_MOVIL, REG_A, 0);
    vm->load(0x1111);
    vm->load(0,0,OP_MOVIL, REG_B, 0);
    vm->load(4);
    vm->load(0,0,OP_MUL, REG_A, REG_B);
    vm->load(0,0,OP_HALT,0);
    vm->reset();
    vm->step();
    vm->step();
    vm->step();
    vm->step();
    assertEquals(vm->get(REG_A), 0x4444, "MUL result"); 
    assert(!vm->getC(), "MUL should not set carry");   
}

void shouldAnd() {
    vm->reset();
    vm->load(0,0,OP_MOVIL, REG_A, 0);
    vm->load(0x1111);
    vm->load(0,0,OP_MOVIL, REG_B, 0);
    vm->load(0x1010);
    vm->load(0,0,OP_AND, REG_A, REG_B);
    vm->load(0,0,OP_HALT,0);
    vm->reset();
    vm->step();
    vm->step();
    vm->step();
    vm->step();
    assertEquals(vm->get(REG_A), 0x1010, "AND result"); 
    assert(!vm->getC(), "AND should not set carry");   
}

void shouldPrintC() {
    vm->addSyscall(0,VMTests_printC);
    vm->reset();
    vm->load(0,0,OP_MOVIL, REG_SP, 0);
    vm->load((uint16_t)0);
    vm->load(0,0,OP_MOVIL, REG_A, 0);
    vm->load('F');
    vm->load(0,0,OP_PUSHD, REG_A, 0);
    vm->load(0,0,OP_SYSCALL, 0);
    vm->load(0,0,OP_HALT,0);
    vm->reset();
    vm->step();
    vm->step();
    vm->step();
    vm->step(); 
   vm->step(); 

    assertEquals(vm->get(REG_SP), 0, "SP should be zero after printC");
}

void shouldLDBI() {
    vm->reset();
    vm->load(0,0,OP_MOVBI, 5);
    vm->reset();
    vm->step();

    assertEquals(vm->get(REG_B), 5, "B should be 5");
}

void shouldAddI() {
    vm->reset();
    vm->load(0,0,OP_MOVBI, 5);
    vm->load(0,0,OP_ADDI, REG_B, -2);
    vm->reset();
    vm->step();
    vm->step();

    assertEquals(vm->get(REG_B), 3, "B should be 3 after add");
}

};
#endif