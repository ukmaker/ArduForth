#ifndef UKMAKER_SYMBOL_H
#define UKMAKER_SYMBOL_H
#include "Token.h"

// Maintain a list of all defined symbols
class Symbol {

    public:

    const char *name;
    Symbol *next;
    Token *token = NULL;

    bool resolved() {
        return token != NULL;
    }
};
#endif