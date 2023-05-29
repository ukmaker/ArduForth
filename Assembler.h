#ifndef UKMAKER_ASSEMBLER_H
#define UKMAKER_ASSEMBLER_H
#include "ForthIS.h"
#include "AssemblyVocabulary.h"
#include "Token.h"
#include "Symbol.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>



class Assembler {

    public:

    Assembler() {
        //vocab.init();
    }
    ~Assembler() {}

    bool slurp(const char *fileName) {

        FILE *file = fopen(fileName, "r");
        size_t n = 0;
        int c;

        if (file == NULL) {
            return false; //could not open file
        }

        fseek(file, 0, SEEK_END);
        sourceLen = ftell(file);
        fseek(file, 0, SEEK_SET);
        source = (char *)malloc(sourceLen);

        while ((c = fgetc(file)) != EOF) {
            source[n++] = (char)c;
        }

        source[n] = '\0'; 
        sourceLen = n-1;

        //fclose(file); 

        line = 1;
        pos = 1;
        lastPos = 1;
        idx = 0;     

        return true;
    }

    long fileSize() {
        return sourceLen;
    }

    /**
     * Scan and tokenize
    */
    void pass1() {

        tokens = NULL;
        symbols = NULL;
        idx = 0;
        line = 1;
        pos = 1;

        tokens = getToken();
        Token *tok = tokens;
        Symbol *sym = symbols;

        while(tok->type != TOKEN_TYPE_EOF && tok->type != TOKEN_TYPE_ERROR) {

            switch(tok->type) {
                case TOKEN_TYPE_CONST:
                case TOKEN_TYPE_LABEL:
                case TOKEN_TYPE_STR:
                case TOKEN_TYPE_VAR:
                    sym = getSymbol(tok->name);
                    if(sym->token != NULL) {
                        printf("ERROR: symbol \"%s\" redefined at line %d (first defined at line %d)",
                            tok->name,
                            tok->line,
                            sym->token->line
                        );
                        return;
                    } else {
                        sym->token = tok;
                    }
                    break;
                default: break;
            } 
            tok->next = getToken();
            tok = tok->next;
        }
        if(tok->type == TOKEN_TYPE_ERROR) {
            printf("ERROR: %s at line %d col %d\n", tok->str, tok->line, tok->pos);
        }
    }

    /**
     * Assign addresses to vars, strings and opcodes
    */
    void pass2() {
        uint16_t addr = 0; // no .org or anything yet. All code starts at address zero
        Token *tok = tokens;
        while(tok != NULL) {
            switch(tok->type) {
                case TOKEN_TYPE_COMMENT: break; // No code to emit
                case TOKEN_TYPE_CONST: break; // No code to emit
                case TOKEN_TYPE_DIRECTIVE: 
                    // only directive currently is ORG
                    addr = tok->value;
                    break;

                case TOKEN_TYPE_VAR:
                    tok->address = addr;
                    addr += 2;
                    break;

                case TOKEN_TYPE_OPCODE: {
                    tok->address = addr;
                    addr += 2;
                    switch(tok->opcode) {
                        case OP_LDL:
                        case OP_STL:
                        case OP_ADDL:
                        case OP_CMPL:
                        case OP_JPIDX:
                        case OP_JP:
                        case OP_CALL:
                            addr += 2;
                            break;
                        default: 
                            break;
                        }
                    }
                    // Resolve symbols on the next pass
                    break;

                case TOKEN_TYPE_STR:
                    tok->address = addr;
                    addr += tok->value;
                    if(addr % 1) addr++; // align to word boundaries
                    break;
                    

                case TOKEN_TYPE_LABEL:
                    tok->address = addr;
                    break;

                case TOKEN_TYPE_EOF: break; 
                case TOKEN_TYPE_ERROR: break;
                default:
                    break;
            }
        
            tok = tok->next;
        }
    }

    /**
     * On this pass we can resolve any symbols
    */
    void pass3() {
        // Resolve all the symbols now that we can reserve space appropriately
        Token *tok = tokens;
        while(tok != NULL) {
            switch(tok->type) {
                case TOKEN_TYPE_OPCODE:
                    if(tok->symbolic) {
                        phase3Error |= dereferenceSymbol(tok);
                    }
                    break;

                case TOKEN_TYPE_VAR:
                case TOKEN_TYPE_STR:
                case TOKEN_TYPE_COMMENT:
                case TOKEN_TYPE_CONST: 
                case TOKEN_TYPE_DIRECTIVE: 
                case TOKEN_TYPE_LABEL:
                case TOKEN_TYPE_EOF:
                case TOKEN_TYPE_ERROR:
                default:
                    break;
            }
            tok = tok->next;
        }
    }

    void dump() {
        Token *tok = tokens;
        Symbol *sym = symbols;

        printf("==============================\n");
        printf("Unresolved Symbols\n");
        printf("==============================\n");
        while(sym != NULL) {
            if(!sym->resolved()) printf("%s\n", sym->name);
            sym = sym->next;
        }
        sym = symbols;
        printf("==============================\n");
        printf("Constants \n");
        printf("==============================\n");
        while(sym != NULL) {
            if(sym->resolved() && sym->token->type == TOKEN_TYPE_CONST) {
                printf("%s = %d\n", sym->name, sym->token->value);
            }
            sym = sym->next;
        }
        sym = symbols;
        printf("==============================\n");
        printf("Variables \n");
        printf("==============================\n");
        while(sym != NULL) {
            if(sym->resolved() && sym->token->type == TOKEN_TYPE_VAR) {
                printf("%s = %d\n", sym->name, sym->token->value);
            }
            sym = sym->next;
        }
        sym = symbols;
        printf("==============================\n");
        printf("Strings \n");
        printf("==============================\n");
        while(sym != NULL) {
            if(sym->resolved() && sym->token->type == TOKEN_TYPE_STR) {
                printf("%s Length %d - %s\n", sym->name, sym->token->value, sym->token->str);
            }
            sym = sym->next;
        }

        printf("==============================\n");
        printf("Code \n");
        printf("==============================\n");
        tok = tokens;
        while(tok != NULL) {
            switch(tok->type) {
                case TOKEN_TYPE_COMMENT: break; // discard comments
                case TOKEN_TYPE_CONST: printf("#%s: %d\n", tok->name, tok->value); break;
                case TOKEN_TYPE_LABEL:  printf("%s: ", tok->name); break;
                case TOKEN_TYPE_OPCODE: printOpcode(tok); break;
                case TOKEN_TYPE_STR:  printf("#%s: %s\n", tok->name, tok->str); break;
                case TOKEN_TYPE_VAR:  printf("$%s: %d\n", tok->name, tok->value); break;
                case TOKEN_TYPE_DIRECTIVE:  printf(".ORG: %d\n", tok->value); break;
                default: break;
            }
            tok = tok->next;
        }
    }

    void writeCode() {
        Token *tok = tokens;
        uint8_t written;
        const char *c;

        while(tok != NULL) {
            switch(tok->type) {

                case TOKEN_TYPE_VAR:
                    printf("%04x %02x %02x\n", tok->address, tok->value & 0xff, (tok->value & 0xff00) >> 8);
                    break;

                case TOKEN_TYPE_OPCODE: {
                    printf("%04x %02x %02x\n", tok->address, tok->lowByte(), tok->highByte());
                    switch(tok->opcode) {
                        case OP_LDL:
                        case OP_STL:
                        case OP_ADDL:
                        case OP_CMPL:
                        case OP_JPIDX:
                        case OP_JP:
                        case OP_CALL:
                            printf("%04x %02x %02x\n", tok->address+2, (tok->value & 0xff00) >> 8, tok->value & 0xff);
                            break;
                        default: 
                            break;
                        }
                    }
                    // Resolve symbols on the next pass
                    break;

                case TOKEN_TYPE_STR: {
                        c = tok->str;
                        written = 0;
                        while(*c != '\0') {
                            if(written % 8 == 0) {
                                printf("\n%04x ", tok->address + written);
                            }
                            printf("%02x ", *c);
                            written++;
                            c++;
                        }
                        printf("\n");
                    }
                    break;

                default:
                    break;
            }
        
            tok = tok->next;
        }
    }

    bool dereferenceSymbol(Token *tok) {

        Symbol *sym;

        switch(tok->opcode) {

            case OP_LDI:
            case OP_STI:
            case OP_ADDI:
            case OP_CMPI:
                // register in arga 3-bit tiny immediates in value
                sym = getSymbol(tok->str);
                if(sym->token == NULL) {
                    printf("ERROR: unknown symbol %s at line %d \n", tok->str, tok->line);
                    return false;
                }
                tok->value = sym->token->value;
                tok->argb = tok->value;
                tok->symbolic = false;
                break;

            // Defined register, 6-bit immediate
            case OP_LDAI:
            case OP_LDBI:
            case OP_STAI:
            case OP_STBI:
            case OP_STAIB:
            case OP_STBIB:
            case OP_ADDAI:
            case OP_ADDBI:
            case OP_CMPAI:
            case OP_CMPBI:
            case OP_JR:
            case OP_CALLR:
            case OP_SYSCALL:
                sym = getSymbol(tok->str);
                if(sym->token == NULL) {
                    printf("ERROR: unknown symbol %s at line %d \n", tok->str, tok->line);
                    return false;
                }                
                tok->value = sym->token->value;
                tok->arga = (tok->value & (7 << 3)) >> 3;
                tok->argb = tok->value & 7;
                tok->symbolic = false;
                break;

            // arga and long immediates in the following word
            case OP_LDL:
            case OP_STL:
            case OP_ADDL:
            case OP_CMPL:
            case OP_JPIDX:
            case OP_JP:
            case OP_CALL:
                sym = getSymbol(tok->str);
                if(sym->token == NULL) {
                    printf("ERROR: unknown symbol %s at line %d \n", tok->str, tok->line);
                    return false;
                }                
                tok->value = sym->token->value;
                tok->symbolic = false;
                break;

            default:
                break;
        } 

        return true;       
    }

    void printOpcode(Token *tok) {
        printf("%s", vocab.opname(tok->opcode));
        if(tok->isConditional()) {
            printf("[");
            if(tok->isConditionNegated()) {
                printf("N");
            }
            printf("%s", vocab.ccname(tok->getCondition()));
            printf("] ");
        } else {
            printf(" ");
        }

        switch(tok->opcode) {

            case OP_LD:
            case OP_CMP:
            case OP_ADD:
            case OP_SUB:
            case OP_MUL:
            case OP_DIV:
            case OP_AND:
            case OP_OR:
            case OP_XOR:
            case OP_STAIX:
            case OP_STBIX:
            case OP_STAIXB:
            case OP_STBIXB:
            case OP_FETCH:
            case OP_ST:
                printf("%s,%s",vocab.argname(tok->arga), vocab.argname(tok->argb));
                break;

            case OP_LDI:
            case OP_STI:
            case OP_ADDI:
            case OP_CMPI:
                // register in arga 3-bit tiny immediates in value
                if(tok->symbolic) {
                    printf("%s,%s",vocab.argname(tok->arga), tok->str);
                } else {
                    printf("%s,%d",vocab.argname(tok->arga), tok->value);
                }
                break;

            // Defined register, 6-bit immediate
            case OP_LDAI:
            case OP_LDBI:
            case OP_STAI:
            case OP_STBI:
            case OP_STAIB:
            case OP_STBIB:
            case OP_ADDAI:
            case OP_ADDBI:
            case OP_CMPAI:
            case OP_CMPBI:
            case OP_JR:
            case OP_CALLR:
            case OP_SYSCALL:
                if(tok->symbolic) {
                    printf("%s", tok->str);
                } else {
                    printf("%d", tok->value);
                }
                break;

            // arga and long immediates in the following word
            case OP_LDL:
            case OP_STL:
            case OP_ADDL:
            case OP_CMPL:
            case OP_JPIDX:
                if(tok->symbolic) {
                    printf("%s,%s",vocab.argname(tok->arga), tok->str);
                } else {
                    printf("%s,%d",vocab.argname(tok->arga), tok->value);
                }
                break;
            // long immediates only
            case OP_JP:
            case OP_CALL:
                if(tok->symbolic) {
                    printf("%s", tok->str);
                } else {
                    printf("%d", tok->value);
                }
                break;

            // arga only
            case OP_PUSHD:
            case OP_POPD:
            case OP_PUSHR:
            case OP_POPR:
            case OP_NOT:
            case OP_SL:
            case OP_SR:
                printf("%s",vocab.argname(tok->arga));
                break;

            default:
                break;
        }

        printf("\n");
    }

    Symbol *getSymbol(const char *name) {
        Symbol *sym = symbols;
        if(symbols == NULL) {
            symbols = new Symbol;
            symbols->name = name;
            sym = symbols;
        } else {
            Symbol *last;
            while(sym != NULL) {
                if(strcmp(sym->name, name) == 0) {
                    return sym;
                }
                last = sym;
                sym = sym->next;
            }
            // nothing found, append a new one
            sym = new Symbol;
            sym->name = name;
            last->next = sym;
        }
        return sym;
    }

    Token *getToken() {

        char c;
        while(idx < sourceLen) {

            skipSpaceOrEOL();
            if(idx >= sourceLen) {
                return Token::eof();
            }

            c = source[idx];
            switch(c) {
                case ' ': inc(); break; // ignore leading spaces
                case '\n': inc(); break; // ignore newlines
                case ';': inc(); return getComment(); break; // rest of the line is a comment
                case '#': inc(); return getConstant(); break;
                case '%': inc(); return getVariable(); break;
                case '$': inc(); return getString(); break;
                case '.': inc(); return getDirective(); break;
                default:
                    if(isLabel()) {
                        return getLabel();
                    } else {
                        return getOpcode();
                    }
                    break;
            }
        }
        return Token::eof();
    }

    void skipSpaces() {
        while(idx < sourceLen) {
            if(isSpace(source[idx])) {
                inc();
            } else {
                break;
            }
        }
    }

    void skipSpaceOrEOL() {
        while(idx < sourceLen) {
            if(isSpaceOrEOL(source[idx])) {
                inc();
            } else {
                break;
            }
        }
    }

    // Returns true if the next token is a label
    // i.e. [_A-Za-z0-9+]:
    bool isLabel() {
        int here = idx;
        
        while(here < sourceLen) {
            char c = source[here];
            if(isAlphaNumeric(c) || isUnderscore(c)) {
                here++;
            } else {
                if(c == ':') {
                    return here > idx;
                }
                return false;
            }
        }
        return false;
    }

    bool isUnderscore(char c) {
        return c == '_';
    }

    bool isAlphaNumeric(char c) {
        return isAlpha(c) || isNumeric(c);
    }

    bool isAlpha(char c) {
        if(c >= 'A' && c <= 'Z') return true;
        if(c >= 'a' && c <= 'z') return true;
        return false;
    }

    bool isNumeric(char c) {
        if(c >= '0' && c <= '9') return true;
        return false;
    }

    bool isSpace(char c) {
        if(c == ' ' || c == '\t') {
            return true;
        }
        return false;
    }

    bool isSpaceOrSemi(char c) {
        if(c == ' ' || c == '\t' || c == ';') {
            return true;
        }
        return false;
    }

    bool isSpaceSemiOrEOL(char c) {
        if(c == ' ' || c == '\t' || c == ';' || c == '\n') {
            return true;
        }
        return false;
    }

    bool isTerminator(char c) {
        const char *terminators = " ;\n";
        const char *p = strchr(terminators, c);
        return p != NULL;
    }

    bool isSpaceOrEOL(char c) {
        const char *spaces = " \n\t";
        const char *p = strchr(spaces, c);
        return p != NULL;
    }

    void inc() {
        if(idx < sourceLen) {
            if(source[idx] == '\n') {
                line++;
                lastPos = pos;
                pos = 1;
            } else {
                pos++;
            }
            idx++;
        }
    }

    void dec() {
        if(idx > 0) {
            idx--;
            if(source[idx] == '\n') {
                pos = lastPos;
                line--;
            }
        }
    }

    Token *getDirective() {
        const char *message = "Directive expected";
        const char *message1 = "Number expected";
        // First char must be alpha
        if(!isAlpha(source[idx])) {
            return Token::error(line,pos,message);
        }
        int here = idx;
        int p = pos - 1;
        inc();
        while(idx < sourceLen) {
            if(isAlpha(source[idx])) {
                inc();
            } else {
                break;
            }
        }

        int dir = vocab.findDirective(source, here, idx-here);
         if(dir == -1) {
            return Token::error(line,pos,message);
        }

        Token *tok = new Token(NULL, line, p);
        tok->type = TOKEN_TYPE_DIRECTIVE;
        tok->opcode = dir;
        if(parseNumber(tok) == -1) {
            return Token::error(line, pos, message1);
        }

        return tok;
    }

    Token *getOpcode() {
        const char *message = "Opcode expected";
        const char *condMessage = "Invalid condition";
        // First char must be alpha
        if(!isAlpha(source[idx])) {
            return Token::error(line,pos,message);
        }
        int here = idx;
        int condition = 0;
        inc();
        while(idx < sourceLen) {
            if(isAlpha(source[idx])) {
                inc();
            } else {
                break;
            }
        }

        int opcode = vocab.findOpcode(source, here, idx-here);
        if(opcode == -1) {
            return Token::error(line,pos,message);
        }

        Token *tok = new Token(NULL, line, pos);
        tok->type = TOKEN_TYPE_OPCODE;
        tok->opcode = opcode;
        if(source[idx] == '[') {
            inc();
            condition = getCondition();
            if(condition == -1) {
                return Token::error(line,pos,condMessage);
            }
        }

        if(condition != 0) {
            tok->condition = condition;
        }
        switch(opcode) {
            case OP_NOP:
            case OP_RET:
            case OP_HALT: 
                break;

            case OP_LD:
            case OP_CMP:
            case OP_ADD:
            case OP_SUB:
            case OP_MUL:
            case OP_DIV:
            case OP_AND:
            case OP_OR:
            case OP_XOR:
            case OP_STAIX:
            case OP_STBIX:
            case OP_STAIXB:
            case OP_STBIXB:
            case OP_FETCH:
            case OP_ST:
                getArgA(tok) && comma(tok) && getArgB(tok); 
                break;

            case OP_LDI:
            case OP_STI:
            case OP_ADDI:
            case OP_CMPI:
                // register in arga 3-bit tiny immediates in value
                getArgA(tok) && comma(tok) && getImm(tok);
                break;

            // Defined register, 6-bit immediate
            case OP_LDAI:
            case OP_LDBI:
            case OP_STAI:
            case OP_STBI:
            case OP_STAIB:
            case OP_STBIB:
            case OP_ADDAI:
            case OP_ADDBI:
            case OP_CMPAI:
            case OP_CMPBI:
            case OP_JR:
            case OP_CALLR:
            case OP_SYSCALL:
                getImm(tok);
                break;

            // arga and long immediates in the following word
            case OP_LDL:
            case OP_STL:
            case OP_ADDL:
            case OP_CMPL:
            case OP_JPIDX:
                getArgA(tok) && getImm(tok);
                break;
            // long immediates only
            case OP_JP:
            case OP_CALL:
                getImm(tok);
                break;

            // arga only
            case OP_PUSHD:
            case OP_POPD:
            case OP_PUSHR:
            case OP_POPR:
            case OP_NOT:
            case OP_SL:
            case OP_SR:
                getArgA(tok);
                break;

            default:
                tok->type = TOKEN_TYPE_ERROR;
                tok->str = message;
        }
        return tok;
    }

    /**
     * Returns the condition code as used in bit 3, negation in bit 2, code in bits 1 and 0
     * Leaves idx pointing at the next character after the ]
     * Returns -1 if an error occurred
    */
   int getCondition() {
    int cc = 8;
    if(source[idx] == 'N') {
        cc += 4;
        inc();
    }

    switch(source[idx]) {
        case 'C': cc += COND_C; break;
        case 'Z': cc += COND_Z; break;
        case 'M': cc += COND_M; break;
        case 'P': cc += COND_P; break;
        default: return -1;
    }
    inc();
    if(source[idx] != ']') {
        return -1;
    }
    inc();

    return cc;
   }

    /**
     * @return True if parsing should continue
    */
    bool comma(Token *tok) {
        const char *message = "Comma expected";
        skipSpaces();
        if(source[idx] != ',') {
            tok->type = TOKEN_TYPE_ERROR;
            tok->str = message;
            return false;
        }
        inc();
        return true;
    }

    // this should reference constants also. TBD
    bool getImm(Token *tok) {
        const char *message = "Number or label expected";
        skipSpaces();
        if(source[idx] == '#' || source[idx] == '$' || source[idx] == '%') {
            if(!getSymbolName(tok)) {
                return false;
            }
        } else if(isAlpha(source[idx])) {
            if(!getSymbolName(tok)) {
                return false;
            }
        } else if(parseNumber(tok) == -1) {
            return error(tok, message);           
        }
        return true;
    }

    // move the name following idx to the token's str
    // set the symoblic flag
    bool getSymbolName(Token *tok) {
        const char *message = "Name expected";
        int here = idx;
        idx++; pos++;
        if(idx == sourceLen) {
            return error(tok, message);
        }
        if(!isAlpha(source[idx])) {
            return error(tok, message);
        }
        idx++; pos++;
        while(idx < sourceLen) {
            if(isSpaceSemiOrEOL(source[idx])) {
                break;
            }
            if(!isAlphaNumeric(source[idx])) {
                return error(tok, message);
            }
            idx++; pos++;
        }
        int len = idx - here;
        char *name = (char*)malloc(len + 1); // +1 for the ending \0
        strncpy(name,&source[idx - len], len);
        name[len] = '\0';
        tok->str = name;  
        tok->symbolic = true;
        return true;     
    }

    bool error(Token *tok, const char *msg) {
        tok->type = TOKEN_TYPE_ERROR;
        tok->str = msg;
        return false;
    }

    bool getArgA(Token *tok) {
        int arg;
        if((arg = getArg(tok)) != -1) {
            tok->arga = arg;
            return true;
        }

        return false;
    }

    bool getArgB(Token *tok) {
        int arg;
        if((arg = getArg(tok)) != -1) {
            tok->argb = arg;
            return true;
        }

        return false;
    }

    int getArg(Token *tok) {
        const char *message = "Register name expected";
        // Any of the register definitions
        skipSpaces();
        int here = idx;
        while(idx < sourceLen) {
            if(!isAlpha(source[idx])) {
                break;
            }
            inc();
        }

        if(idx == here) {
            tok->type = TOKEN_TYPE_ERROR;
            tok->str = message;
            return -1;
        }
        int arg = vocab.findArg(source, here, idx - here);
        if(arg == -1) {
            tok->type = TOKEN_TYPE_ERROR;
            tok->str = message;
            return -1;
        }

        return arg;
   } 

    Token *getComment() {

        Token *tok = new Token(NULL,line,pos);
        
        int len;
        len = moveTo('\n');
        if(len == -1) {
            return Token::eof();
        }
        // append to end of line
        char *comment = (char*)malloc(len + 1); // +1 for the ending \0
        strncpy(comment,&source[idx - len], len);
        comment[len] = '\0';
        tok->str = comment;
        tok->type = TOKEN_TYPE_COMMENT;
        tok->pos -= 1;
        return tok;
    }

    Token *getConstant() {
        const char *message = "Number expected";
        Token *tok = _getLabel(TOKEN_TYPE_CONST, ':'); 
        inc();
        if(parseNumber(tok) == -1) {
            return Token::error(line,pos,message);
        }
        return tok;
    }

    Token *getVariable() {
        const char *message = "Number expected";
        Token *tok = _getLabel(TOKEN_TYPE_VAR, ':'); 
        inc();
        if(parseNumber(tok) == -1) {
            return Token::error(line,pos,message);
        }
        return tok;              
    }

    Token *getLabel() {
        Token *tok = _getLabel(TOKEN_TYPE_LABEL, ':', false); 
        inc();
        return tok;     
    }

    Token *getString() {
        const char *message = "String expected";
        Token *tok = _getLabel(TOKEN_TYPE_STR, ':'); 
        inc();
        if(parseString(tok) == -1) {
            return Token::error(line, pos, message);
        } 
        return tok; 
    }

    Token *_getLabel(uint8_t type, char sep, bool typed = true) {

        int len;
        len = moveTo(sep);
        if(len == -1) {
            return Token::eof();
        }

        if(typed) len++;

        Token *tok = new Token(NULL,line,pos - len);        
        char *name = (char*)malloc(len + 1);
        strncpy(name,&source[idx-len], len);
        name[len] = '\0';
        tok->name = name;
        tok->type = type;
        return tok;         
    }

    /**
     * Fill in the token's value
     * @return 0 on succes, -1 on some sort of failure
    */
    int parseNumber(Token *tok) {
        // Allowed formats
        // 93 - decimal
        // 0x3A - hex
        // 0b0110 - binary
        // 'C' - a single character
        char c;
        while(idx < sourceLen) {
            c = source[idx];
            switch(c) {
                case '\'':
                    inc();
                    if(idx >= sourceLen) {
                        return -1;
                    }
                    // backslash is allowed
                    if(source[idx] == '\\') {
                        inc();
                        if(idx >= sourceLen) {
                            return -1;
                        }
                        c = parseEscapedChar(source[idx]); 
                    }
                    break;
                case '0': 
                    inc();
                    if(idx >= sourceLen) {
                        return -1;
                    }
                    if(source[idx] == 'x') {
                        inc();
                        return parseHex(tok);
                    }
                    
                    if(source[idx] == 'b') {
                        inc();
                        return parseBin(tok);
                    }
                    // fallthrough
                    dec();
                    [[fallthrough]];
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    return parseDec(tok);

                case ' ':
                case '\t':
                    // leading spaces and tabs are allowed
                    inc();
                    break;

                default:
                    return -1;
            }
        }
        return -1;
    }

    char parseEscapedChar(char c) {
        switch(c) {
            case 'n': return '\n';
            case '0': return 0;
            case 'b': return '\b';
            case 'r': return '\r';
            case 't': return '\t';
            case '\\': return '\\';
            case '\'': return '\'';
            case '"': return '"';
            default: return c;
        }
    }

    int parseString(Token *tok) {
        int c;
        int len;
        while((c = getChar()) != -1) {
            if(c == '"') {
                len = moveTo('"');       
                char *str = (char*)malloc(len + 1);
                strncpy(str,&source[idx-len], len);
                str[len] = '\0';
                tok->str = str;
                tok->value = len;
                inc();
                return 0;                
            } else if(c != ' ') {
                return -1;
            }
        }
        return -1;
    }

    /*
    * @return true on success
    * @return false on failure and mutate the Token to an error
    */
    bool parseHex(Token *tok) {
        tok->value = 0;
        int c = getChar();

        if(c == -1) {
            return hexError(tok);
        }

        int hv = hexVal(c);
        if(hv == -1) {
            return hexError(tok);
        }

        tok->value = hv;
        // can be up to 3 more hex chars before the separator
        for(uint8_t i=0; i<3; i++) {
            c = getChar();
            if(c == -1) {
                // EOF is allowed
                return true;
            }
            if(isTerminator(c)) {
                return true;
            }
            hv = hexVal(c);
            if(hv == -1) {
                return hexError(tok);              
            }
            tok->value = (tok->value * 16) + hv;
        }
        // next char must be EOF or a valid terminator
        if(idx >= sourceLen) {
            return true;
        }
        if(isTerminator(source[idx])) {
            return true;
        }

        return hexError(tok);
    }

    bool hexError(Token *tok) {
        const char *message  = "Hex number expected";
        tok->type = TOKEN_TYPE_ERROR;
        tok->str = message;
        return false;  
    }

    int hexVal(char c) {
        const char *hexen = "0123456789ABCDEFabcdef";
        const char *p = strchr(hexen, c);
        if(p != NULL) {
            uint8_t l = p - hexen;
            if(l > 15) l = l - 6;
            return l;
        }
        return -1;
    }

    /*
    * @return true on success
    * @return false on failure and mutate the Token to an error
    */
    bool parseBin(Token *tok) {
        tok->value = 0;
        int c = getChar();

        if(c == -1) {
            return binError(tok);
        }

        int hv = binVal(c);
        if(hv == -1) {
            return binError(tok);
        }

        tok->value = hv;
        // can be up to 15 more bits before the separator
        for(uint8_t i=0; i<15; i++) {
            c = getChar();
            if(c == -1) {
                // EOF is allowed
                return true;
            }
            if(isTerminator(c)) {
                return true;
            }
            hv = binVal(c);
            if(hv == -1) {
                return binError(tok);              
            }
            tok->value = (tok->value * 2) + hv;
        }
        // next char must be EOF or a valid terminator
        if(idx >= sourceLen) {
            return true;
        }
        if(isTerminator(source[idx])) {
            return true;
        }

        return binError(tok);
    }

    bool binError(Token *tok) {
        const char *message  = "Binary number expected";
        tok->type = TOKEN_TYPE_ERROR;
        tok->str = message;
        return false;  
    }

    int binVal(char c) {
        const char *hexen = "01";
        const char *p = strchr(hexen, c);
        if(p != NULL) {
            uint8_t l = p - hexen;
            return l;
        }
        return -1;
    }



    /*
    * @return true on success
    * @return false on failure and mutate the Token to an error
    */
    bool parseDec(Token *tok) {
        tok->value = 0;
        int c = getChar();

        if(c == -1) {
            return decError(tok);
        }

        int hv = decVal(c);
        if(hv == -1) {
            return decError(tok);
        }

        tok->value = hv;
        // can be up to 5 more digits before the separator
        for(uint8_t i=0; i<5; i++) {
            c = getChar();
            if(c == -1) {
                // EOF is allowed
                return true;
            }
            if(isTerminator(c)) {
                return true;
            }
            hv = decVal(c);
            if(hv == -1) {
                return decError(tok);              
            }
            tok->value = (tok->value * 10) + hv;
        }
        // next char must be EOF or a valid terminator
        if(idx >= sourceLen) {
            return true;
        }
        if(isTerminator(source[idx])) {
            return true;
        }

        return decError(tok);
    }

    bool decError(Token *tok) {
        const char *message  = "Decimal number expected";
        tok->type = TOKEN_TYPE_ERROR;
        tok->str = message;
        return false;  
    }

    int decVal(char c) {
        const char *decs = "0123456789";
        const char *p = strchr(decs, c);
        if(p != NULL) {
            uint8_t l = p - decs;
            return l;
        }
        return -1;
    }

    int getChar() {
        if(idx < sourceLen) {
            char c = source[idx];
            inc();
            return c;
        }
        return -1;
    }

    /*
    * @return the length of the string
    * 0 if there is no string
    * -1 if the end of the file is reached
    * Leaves the idx pointing at the separator
    * */
    int moveTo(char sep) {
        // look forward until the given char, or end of line, or end of file
        char c;
        int len=0;
        bool found = false;
        while(idx < sourceLen) {
            c = source[idx];
            if((c == sep) || (c == '\n'))  {
                break;
            }
            found = true;
            inc();
            len++;
        }

        if(found) {
            return len;
        }

        if(idx >= sourceLen) {
            return -1;
        }

        return 0; 
    }

    /**
     * @return a new Token or NULL if a token with the given name already exists
    */
    Token *append(char *name, int line, int pos) {
        if(tokens == NULL) {
            // This is the first token
            tokens = new Token(name, line, pos);
            return tokens;
        }

        // Search the list to see if a match already exists
        Token *tok = tokens;
        bool more = true;
        while(more) {
            if(tok->isNamed(name)) {
                printf("Token %s at line %d pos %d redefined at line %d pos %d",
                name, tok->line, tok->pos, line, pos);
                return NULL;
            }
            if(tok->next != NULL) {
                tok = tok->next;
            } else {
                more = false;
            }
        }

        Token *last = new Token(name, line, pos);
        tok->next = last;
        return last;
    }

    protected:

    char *source;
    long sourceLen;
    long idx;
    long line;
    long pos;
    long lastPos;
    long addr;
    Token *tokens = NULL;
    Symbol *symbols = NULL;
    AssemblyVocabulary vocab;
    bool phase1Error;
    bool phase2Error;
    bool phase3Error;

};
#endif