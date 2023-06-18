#ifndef UKMAKER_CONSTANT_TESTS_H
#define UKMAKER_CONSTANT_TESTS_H

#include "Test.h"

class ConstantTests : public Test {
    public:
    ConstantTests(TestSuite *suite, ForthVM *fvm, Assembler *vmasm) : Test(suite, fvm, vmasm) {}
    void run() {
        shouldOpenTestConstants();
        skipTokens(1);
        shouldGetAConstant("#SYSCALL_DOT", 1);
        shouldGetAConstant("#SYSCALL_DOT_C", 2);
        fasm->pass1();
        fasm->pass2();
        fasm->pass3();
    }

    void shouldOpenTestConstants() {
        shouldOpenAsmFile("tests/test-constants.fasm", 86);
    }


};
#endif