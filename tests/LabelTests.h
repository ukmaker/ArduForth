#ifndef UKMAKER_LABELTESTS_H
#define UKMAKER_LABELTESTS_H

#include "Test.h"

class LabelTests : public Test {
    public:
    LabelTests(TestSuite *suite, ForthVM *fvm, Assembler *vmasm, Loader *loader) : Test(suite, fvm, vmasm, loader) {}
    void run() {
        shouldOpenTestLabels();
        skipTokens(1);
        shouldGetALabel("TOKEN");
        skipTokens(3);
        shouldGetALabel("TOKEN_WA");
        skipTokens(1);
        shouldGetALabel("TOKEN_CA");
        skipTokens(10);
        shouldGetAnOpcode("PUSHR", OP_PUSHR);
    }

    void shouldOpenTestLabels() {
        shouldOpenAsmFile("tests/test-labels.asm", 70);
    }


};
#endif