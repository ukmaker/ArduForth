#ifndef UKMAKER_DUMPER_H
#define UKMAKER_DUMPER_H

#include "Assembler.h"

class Dumper {

    public:
    Dumper() {}
    ~Dumper() {}

    void writeCPP(Assembler *fasm, size_t romSize) {
        FILE *fp = fopen("ForthImage.h", "w");
        fprintf(fp, "#ifndef UKMAKER_FORTH_IMAGE_H\n");
        fprintf(fp, "#define UKMAKER_FORTH_IMAGE_H\n");
        fprintf(fp,"/******************************\n");
        fprintf(fp, "* Constants\n");
        fprintf(fp, "*****************************/\n");
        Token *tok = fasm->tokens;
        while(tok != NULL) {
            if(tok->isConst()) {
                printf("#define %s %04x\n", tok->name, tok->value); 
            }
            tok = tok->next;
        }
        // Write the ROM image
        fprintf(fp, "const char rom[%d] = {\n",romSize);

        fprintf(fp, "#endif // UKMAKER_FORTH_IMAGE_H\n");
        

        fclose(fp);
/**

                    break;
                case TOKEN_TYPE_LABEL:  
                    printf("%04x ", tok->address);
                    printf("%s: \n", tok->name); 
                    break;
                case TOKEN_TYPE_OPCODE: 
                    printf("%04x   ", tok->address);
                    printOpcode(fasm, tok);
                    printf("\n"); 
                    break;
                case TOKEN_TYPE_STR:  
                    printf("%04x ", tok->address);
                    printf("%s: %s\n", tok->name, tok->str); 
                    break;
                case TOKEN_TYPE_VAR:  
                    printf("%04x ", tok->address);
                    printf("%s: %04x\n", tok->name, tok->value); 
                    break;
                case TOKEN_TYPE_DIRECTIVE:  
                    
                    switch(tok->opcode) {
                        case DIRECTIVE_TYPE_ORG: printf(".ORG: %04x\n", tok->value); break;
                        case DIRECTIVE_TYPE_DATA: 
                            printf("%04x ", tok->address);
                            printf(".DATA: %04x\n", tok->value); 
                            break;
                        case DIRECTIVE_TYPE_SDATA: 
                            printf("%04x ", tok->address);
                            printf(".SDATA: \"%s\"\n", tok->str); 
                            break;
                        default: break;
                    }
                break;
                default: break;
            }
            tok = tok->next;
        }  
            **/
    }

    void dump(Assembler *fasm) {

        printf("==============================\n");
        printf("Code \n");
        printf("==============================\n");
        Token *tok = fasm->tokens;
        while(tok != NULL) {
            switch(tok->type) {
                case TOKEN_TYPE_COMMENT: break; // discard comments
                case TOKEN_TYPE_CONST: 
                    printf("%s: %04x\n", tok->name, tok->value); 
                    break;
                case TOKEN_TYPE_LABEL:  
                    printf("%04x ", tok->address);
                    printf("%s: \n", tok->name); 
                    break;
                case TOKEN_TYPE_OPCODE: 
                    printf("%04x   ", tok->address);
                    printOpcode(fasm, tok);
                    printf("\n"); 
                    break;
                case TOKEN_TYPE_STR:  
                    printf("%04x ", tok->address);
                    printf("%s: %s\n", tok->name, tok->str); 
                    break;
                case TOKEN_TYPE_VAR:  
                    printf("%04x ", tok->address);
                    printf("%s: %04x\n", tok->name, tok->value); 
                    break;
                case TOKEN_TYPE_DIRECTIVE:  
                    
                    switch(tok->opcode) {
                        case DIRECTIVE_TYPE_ORG: printf(".ORG: %04x\n", tok->value); break;
                        case DIRECTIVE_TYPE_DATA: 
                            printf("%04x ", tok->address);
                            printf(".DATA: %04x\n", tok->value); 
                            break;
                        case DIRECTIVE_TYPE_SDATA: 
                            printf("%04x ", tok->address);
                            printf(".SDATA: \"%s\"\n", tok->str); 
                            break;
                        default: break;
                    }
                break;
                default: break;
            }
            tok = tok->next;
        }
    }

    void printOpcode(Assembler *fasm, Token *tok) {
        printf("%s", fasm->vocab.opname(tok->opcode));
        if(tok->isConditional()) {
            printf("[");
            if(tok->isConditionNegated()) {
                printf("N");
            }
            printf("%s", fasm->vocab.ccname(tok->getCondition()));
            printf("] ");
        } else {
            printf(" ");
        }

        switch(tok->opcode) {

            case OP_MOV:
            case OP_LD:
            case OP_LD_B:
            case OP_ST:
            case OP_ST_B:
            case OP_CMP:
            case OP_ADD:
            case OP_SUB:
            case OP_MUL:
            case OP_DIV:
            case OP_AND:
            case OP_OR:
            case OP_XOR:
            case OP_SL:
            case OP_SR:
            case OP_RL:
            case OP_RR:
            case OP_RLC:
            case OP_RRC:
            case OP_BIT:
            case OP_SET:
            case OP_CLR:
                printf("%s,%s",fasm->vocab.argname(tok->arga), fasm->vocab.argname(tok->argb));
                break;

            case OP_MOVI:
            case OP_LDAX:
            case OP_LDBX:
            case OP_LDAX_B:
            case OP_LDBX_B:
            case OP_STI:
            case OP_STI_B:
            case OP_STXA:
            case OP_STXB:
            case OP_STXA_B:
            case OP_STXB_B:
            case OP_ADDI:
            case OP_SUBI:
            case OP_CMPI:

            case OP_SLI:
            case OP_SRI:
            case OP_RLI:
            case OP_RRI:
            case OP_RLCI:
            case OP_RRCI:
            case OP_BITI:
            case OP_SETI:
            case OP_CLRI:

            case OP_JX:
            case OP_CALLX:
                // register in arga 4-bit tiny immediates in value
                if(tok->symbolic) {
                    printf("%s,%s",fasm->vocab.argname(tok->arga), tok->str);
                } else {
                    printf("%s,%01x",fasm->vocab.argname(tok->arga), tok->value);
                }
                break;

            // Defined register, 8-bit immediate
            case OP_MOVAI:
            case OP_MOVBI:
            case OP_STAI:
            case OP_STBI:
            case OP_STAI_B:
            case OP_STBI_B:
            case OP_ADDAI:
            case OP_ADDBI:
            case OP_CMPAI:
            case OP_CMPBI:
            case OP_SUBAI:
            case OP_SUBBI:
            case OP_SYSCALL:
            case OP_JR:
            case OP_CALLR:
                if(tok->symbolic) {
                    printf("%s", tok->str);
                } else {
                    printf("%0x2", tok->value);
                }
                break;

            // arga and long immediates in the following word
            case OP_MOVIL:
            case OP_STIL:
            case OP_ADDIL:
            case OP_SUBIL:
            case OP_CMPIL:
                if(tok->symbolic) {
                    printf("%s,%s",fasm->vocab.argname(tok->arga), tok->str);
                } else {
                    printf("%s,%04x",fasm->vocab.argname(tok->arga), tok->value);
                }
                break;
            // long immediates only
            case OP_JP:
            case OP_CALL:
                if(tok->symbolic) {
                    printf("%s", tok->str);
                } else {
                    printf("%04x", tok->value);
                }
                break;

            // arga only
            case OP_PUSHD:
            case OP_POPD:
            case OP_PUSHR:
            case OP_POPR:
            case OP_NOT:
                printf("%s",fasm->vocab.argname(tok->arga));
                break;

            default:
                break;
        }
    }

};
#endif