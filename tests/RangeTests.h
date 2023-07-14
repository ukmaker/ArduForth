#ifndef UKMAKER_RANGETESTS_H
#define UKMAKER_RANGETESTS_H

#include "Test.h"

class RangeTests : public Test
{

public:

    RangeTests(TestSuite *suite, ForthVM *fvm, Assembler *vmasm, Loader *loader) : Test(suite, fvm, vmasm, loader) {}
    
    void run()
    {
        shouldOpenTestRange();
        shouldGetOpeningComment();
        skipTokens(3);
        shouldGetMOVI();
    }

    void shouldOpenTestRange()
    {
        shouldOpenAsmFile("tests/test-range.asm", 70);
    }
    void shouldGetOpeningComment()
    {
        printf("         shouldGetOpeningComment\n");
        Token *tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize a standalone comment");
        assertEquals(tok->type, TOKEN_TYPE_COMMENT, "Type should be COMMENT");
        assertEquals(tok->line, 1, "First comment is on line 1");
        assertEquals(tok->pos, 2, "First comment is on pos 2");
    }
    void shouldGetMOVI()
    {
        printf("         shouldGetMOVI\n");
        Token *tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize an opcode");
        assertEquals(tok->type, TOKEN_TYPE_OPCODE, "Type should be OPCODE");
        assertEquals(tok->opcode, OP_MOVI, "Should be a MOVI instruction");
    }
};
#endif