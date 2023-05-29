#ifndef UKMAKER_TOKEN_H
#define UKMAKER_TOKEN_H

#include <stdint.h>
#include <string.h>
#include "ForthIS.h"

#define TOKEN_TYPE_OPCODE 0
#define TOKEN_TYPE_LABEL 1
#define TOKEN_TYPE_CONST 2
#define TOKEN_TYPE_VAR 3
#define TOKEN_TYPE_STR 4
#define TOKEN_TYPE_COMMENT 5
#define TOKEN_TYPE_ERROR 6
#define TOKEN_TYPE_EOF 7
#define TOKEN_TYPE_DIRECTIVE 8 // e.g. ORG for setting start location in memory

/*
Syntax:
#CNAME: 0x27 ; A constant definition
%VNAME: 0x30 ; A variable definition. Reserves space and sets initial value
$SNAME: "This is a string" ; Stored as a Forth-string <len><str>

LABEL: JR NZ,#3 ; Jump with literal
LABEL: JR NZ,%CONST ; Jump with literal value defined as $CONST:
LABEL: JR NZ,$VAR ; Jump to address of VAR. Emits a warning
LABEL: JR NZ,*LABEL ; Relative jump to the label. Error if the destination is too far
*/

class Token {
    public:
    Token(char *name, int line, int pos) {
        this->name = name;
        this->line = line;
        this->pos = pos;
    }

    ~Token() {}

    static Token *eof() {
        Token *t = new Token(NULL,0,0);
        t->type = TOKEN_TYPE_EOF;
        return t;
    }

    static Token *error(int line, int pos, const char *message) {
        Token *t = new Token(NULL,line,pos);
        t->type = TOKEN_TYPE_ERROR;
        t->str = message;
        return t;
    }

    const char *name;
    int line;
    int pos;

    uint8_t type;

    uint16_t address;
    int value; // NULL for a label; instruction; const or variable value; length of a string
    const char *str;

    // FOr an instruction
    uint8_t opcode;
    uint8_t arga, argb;
    uint8_t condition;
    uint8_t apply;

    // Instructions with immediate values may need to
    // have the value resolved
    // num3 and num6 values may be taken from a constant definition
    // num16 may be a constant or the address of a variable or label
    bool symbolic = false;


    Token *next = NULL;

    bool isConditional() {
        return (condition & 8) != 0;
    }

    bool isConditionNegated() {
         return (condition & 4) != 0;       
    }

    uint8_t getCondition() {
        return condition & 3;
    }

    bool isNamed(char *n) {
        return strcasecmp(name, n) == 0;
    }

    bool isCode() {
        return type == TOKEN_TYPE_OPCODE;
    }

    bool isLabel() {
        return type == TOKEN_TYPE_LABEL;
    }

    bool isConst() {
        return type == TOKEN_TYPE_CONST;
    }

    bool isVar() {
        return type == TOKEN_TYPE_VAR;
    }

    bool isStr() {
        return type == TOKEN_TYPE_STR;
    }

    bool isComment() {
        return type == TOKEN_TYPE_COMMENT;
    }

    uint16_t opWord() {
        uint16_t w = opcode << OP_BITS;
        w |= (condition & 3) << CC_BITS;
        w |= ((condition & 0x0c) >> 2) << CC_APPLY_BITS;
        w |= (arga & 7) << 3;
        w |= argb & 7;
        return w;
    }

    uint8_t lowByte() {
        return opWord() & 0xff;
    }

    uint8_t highByte() {
        return (opWord() & 0xff00) >> 8;
    }
};

#endif