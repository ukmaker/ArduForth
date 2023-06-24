#ifndef UKMAKER_ALIAS_TESTS_H
#define UKMAKER_ALIAS_TESTS_H

#include "Test.h"

class AliasTests : public Test {
    public:
    AliasTests(TestSuite *suite, ForthVM *fvm, Assembler *vmasm) : Test(suite, fvm, vmasm) {}
    void run() {
        shouldOpenTestAliases();
        skipTokens(4);
        shouldGetAnAlias(REG_0, "DP");
        shouldGetAnAlias(REG_1, "*DP");

        fasm->pass1();
        fasm->pass2();
        fasm->pass3();
    }

    void shouldOpenTestAliases() {
        shouldOpenAsmFile("tests/test-aliases.fasm", 86);
    }

    void shouldGetMOVIL() {
        printf("         shouldGetMOVIL\n");
        Token *tok = fasm->getToken();
        assertEquals(tok->type, TOKEN_TYPE_OPCODE, "Type should be OPCODE");
        assertEquals(tok->opcode, OP_MOVIL, "Should be a MOVIL instruction");
    }


};
#endif