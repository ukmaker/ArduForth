#ifndef UKMAKER_SLURPTESTS_H
#define UKMAKER_SLURPTESTS_H

#include "Test.h"

class SlurpTests : public Test {

    public:
    SlurpTests(TestSuite *suite, ForthVM *fvm, Assembler *vmasm) : Test(suite, fvm, vmasm) {}

    void run() {

        testSuite->reset();

        shouldOpenTestSlurp();

        shouldGetOpeningComment();
        shouldGetSecondComment();

        shouldGetHexConstant();
        shouldGetDecConstant();
        shouldGetBinConstant();
        shouldGetOrg1();
        shouldGetVariable();
        shouldGetString();

        shouldGetData1();
        shouldGetData2();
        shouldGetData3();

        shouldGetOrg2();
        shouldGetLabel();
        shouldGetMOV();
        shouldGetNOP();
        shouldGetLabel1();
        shouldGetJP();




        printf("==============================\n");
        printf("TOTAL: %d  PASSED %d  FAILED %d\n", testSuite->tests, testSuite->passed, testSuite->failed);
        printf("==============================\n");
    }


    void shouldOpenTestSlurp() {
        shouldOpenAsmFile("tests/test-slurp.asm", 945);
    }


    void shouldGetOpeningComment() {
        printf("         shouldGetOpeningComment\n");
        Token *tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize a standalone comment");
        assertEquals(tok->type, TOKEN_TYPE_COMMENT, "Type should be COMMENT");
        assertEquals(tok->line, 1, "First comment is on line 1");
        assertEquals(tok->pos, 2, "First comment is on pos 2");
    }

    void shouldGetSecondComment() {
        printf("         shouldGetSecondComment\n");
        Token *tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize a standalone comment");
        assertEquals(tok->type, TOKEN_TYPE_COMMENT, "Type should be COMMENT");
        assertEquals(tok->line, 2, "Second comment is on line 2");
        assertEquals(tok->pos, 1, "Second comment is on pos 1");
    }

    void shouldGetHexConstant() {
        printf("         shouldGetHexConstant\n");
        Token *tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize a HEX constant");
        assertEquals(tok->type, TOKEN_TYPE_CONST, "Type should be CONST");
        assertEquals(tok->line, 5, "Constant is on line 5");
        assertEquals(tok->pos, 1, "Constant is on pos 1");
        assertEquals(strcmp(tok->name, "#HNAME"), 0, "Should parse CONST label");
        assertEquals(tok->value, 0xFA90, "Should parse a hex value");
        // and we should parse the comment at the end of the line
        tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize an end of line comment");
        assertEquals(tok->type, TOKEN_TYPE_COMMENT, "Type should be COMMENT");
    }
    void shouldGetDecConstant() {
        printf("         shouldGetDecConstant\n");
        Token *tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize a DEC constant");
        assertEquals(tok->type, TOKEN_TYPE_CONST, "Type should be CONST");
        assertEquals(tok->line, 6, "Constant is on line 6");
        assertEquals(tok->pos, 1, "Constant is on pos 1");
        assertEquals(strcmp(tok->name, "#DNAME"), 0, "Should parse CONST label");
        assertEquals(tok->value, 3900, "Should parse a decimal value");
        // and we should parse the comment at the end of the line
        tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize an end of line comment");
        assertEquals(tok->type, TOKEN_TYPE_COMMENT, "Type should be COMMENT");
    }
    void shouldGetBinConstant() {
        printf("         shouldGetBinConstant\n");
        Token *tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize a BIN constant");
        assertEquals(tok->type, TOKEN_TYPE_CONST, "Type should be CONST");
        assertEquals(tok->line, 7, "Constant is on line 7");
        assertEquals(tok->pos, 1, "Constant is on pos 1");
        assertEquals(strcmp(tok->name, "#BNAME"), 0, "Should parse CONST label");
        assertEquals(tok->value, 0b10010110, "Should parse a binary value");
        // and we should parse the comment at the end of the line
        tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize an end of line comment");
        assertEquals(tok->type, TOKEN_TYPE_COMMENT, "Type should be COMMENT");
    }

    void shouldGetVariable() {
        printf("         shouldGetVariable\n");
        Token *tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize a variable");
        assertEquals(tok->type, TOKEN_TYPE_VAR, "Type should be VAR");
        assertEquals(tok->line, 10, "Var is on line 10");
        assertEquals(tok->pos, 1, "Var is on pos 1");
        assertString(tok->name, "%VNAME", "Should parse VAR label");
        // and we should parse the comment at the end of the line
        tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize an end of line comment");
        assertEquals(tok->type, TOKEN_TYPE_COMMENT, "Type should be COMMENT");
    }

    void shouldGetString() {
        printf("         shouldGetString\n");
        Token *tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize a string label");
        assertEquals(tok->type, TOKEN_TYPE_STR, "Type should be STRING");
        assertEquals(tok->line, 11, "String is on line 11");
        assertEquals(tok->pos, 1, "String is on pos 1");
        assertString(tok->name, "$SNAME", "Should parse String label");
        assertString(tok->str, "This is a string", "Should have the correct string value");
        // and we should parse the comment at the end of the line
        tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize an end of line comment");
        assertEquals(tok->type, TOKEN_TYPE_COMMENT, "Type should be COMMENT");
    }


    void shouldGetOrg1() {
        printf("         shouldGetOrg1\n");
        Token *tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize a line label");
        assertEquals(tok->type, TOKEN_TYPE_DIRECTIVE, "Type should be DIRECTIVE");
        assertEquals(tok->line, 9, "ORG is on line 9");
        assertEquals(tok->pos, 1, "ORG is on pos 1");
        assertEquals(tok->value, 1000, "Should parse value");
        // and we should parse the comment at the end of the line
        tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize an end of line comment");
        assertEquals(tok->type, TOKEN_TYPE_COMMENT, "Type should be COMMENT");
    }

    void shouldGetOrg2() {
        printf("         shouldGetOrg2\n");
        Token *tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize a line label");
        assertEquals(tok->type, TOKEN_TYPE_DIRECTIVE, "Type should be DIRECTIVE");
        assertEquals(tok->line, 17, "ORG is on line 17");
        assertEquals(tok->pos, 1, "ORG is on pos 1");
        assertEquals(tok->value, 0, "Should parse value");
        // and we should parse the comment at the end of the line
        tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize an end of line comment");
        assertEquals(tok->type, TOKEN_TYPE_COMMENT, "Type should be COMMENT");
    }

    void shouldGetData1() {
        printf("         shouldGetData1\n");
        Token *tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize a directive");
        assertEquals(tok->type, TOKEN_TYPE_DIRECTIVE, "Type should be DIRECTIVE");
        assertEquals(tok->line, 13, "DATA is on line 13");
        assertEquals(tok->pos, 1, "DATA is on pos 1");
        assertEquals(tok->value, 0xfa71, "Should parse value");
        // and we should parse the comment at the end of the line
        tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize an end of line comment");
        assertEquals(tok->type, TOKEN_TYPE_COMMENT, "Type should be COMMENT");
    }
    void shouldGetData2() {
        printf("         shouldGetData2\n");
        Token *tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize a directive");
        assertEquals(tok->type, TOKEN_TYPE_DIRECTIVE, "Type should be DIRECTIVE");
        assertEquals(tok->line, 14, "SDATA is on line 14");
        assertEquals(tok->pos, 1, "SDATA is on pos 1");
        assertString(tok->str, "Hello World!", "Should parse value");
        // and we should parse the comment at the end of the line
        tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize an end of line comment");
        assertEquals(tok->type, TOKEN_TYPE_COMMENT, "Type should be COMMENT");
    }
    void shouldGetData3() {
        printf("         shouldGetData3\n");
        Token *tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize a directive");
        assertEquals(tok->type, TOKEN_TYPE_DIRECTIVE, "Type should be DIRECTIVE");
        assertEquals(tok->line, 15, "DATA is on line 15");
        assertEquals(tok->pos, 1, "DATA is on pos 1");
        assertEquals(tok->value, 0, "Should defer value");
    }

    void shouldGetLabel() {
        printf("         shouldGetLabel\n");
        Token *tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize a line label");
        assertEquals(tok->type, TOKEN_TYPE_LABEL, "Type should be LABEL");
        assertEquals(tok->line, 18, "Label is on line 18");
        assertEquals(tok->pos, 1, "label is on pos 1");
        assertString(tok->name, "START", "Should parse label");
    }

    void shouldGetMOV() {
        printf("         shouldGetMOV\n");
        Token *tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize an opcode");
        assertEquals(tok->type, TOKEN_TYPE_OPCODE, "Type should be OPCODE");
        assertEquals(tok->opcode, OP_MOV, "Should be a MOV instruction");
        // and we should parse the comment at the end of the line
        tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize an end of line comment");
        assertEquals(tok->type, TOKEN_TYPE_COMMENT, "Type should be COMMENT");
    }

    void shouldGetNOP() {
        printf("         shouldGetNOP\n");
        Token *tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize an opcode");
        assertEquals(tok->type, TOKEN_TYPE_OPCODE, "Type should be OPCODE");
        assertEquals(tok->opcode, OP_NOP, "Should be a NOP instruction");
    }

    void shouldGetLabel1() {
        printf("         shouldGetLabel1\n");
        Token *tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize a line label");
        assertEquals(tok->type, TOKEN_TYPE_LABEL, "Type should be LABEL");
        assertEquals(tok->line, 20, "Label is on line 20");
        assertEquals(tok->pos, 1, "label is on pos 1");
        assertString(tok->name, "LABEL1", "Should parse label");
    }

    void shouldGetJP() {
        printf("         shouldGetJP\n");
        Token *tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize an opcode");
        assertEquals(tok->type, TOKEN_TYPE_OPCODE, "Type should be OPCODE");
        assertEquals(tok->opcode, OP_JP, "Should be a JP instruction");
        // and we should parse the comment at the end of the line
        tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize an end of line comment");
        assertEquals(tok->type, TOKEN_TYPE_COMMENT, "Type should be COMMENT");
    }
    void shouldGetMOVI() {
        printf("         shouldGetMOVI\n");
        Token *tok = fasm->getToken();
        assert(tok->type != TOKEN_TYPE_ERROR, "Should tokenize an opcode");
        assertEquals(tok->type, TOKEN_TYPE_OPCODE, "Type should be OPCODE");
        assertEquals(tok->opcode, OP_MOVI, "Should be a MOVI instruction");
    }

};

#endif