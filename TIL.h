#ifndef UKMAKER_TIL_H
#define UKMAKER_TIL_H

#include "ForthVM.h"
#include "Serial.h"
#include <string.h>

#define SYSCALL_PRINTC 0
#define SYSCALL_TYPE 1
#define SYSCALL_DOT 2
#define SYSCALL_GETC 3
#define SYSCALL_PUTC 4
#define SYSCALL_INLINE 5
#define SYSCALL_FLUSH 6

void syscall_printC(ForthVM *vm) {
    // Syscall to print the char on the top of the stack
    // ( c - )
    uint16_t v = vm->pop();
    char c = (char) v;

    printf("%c", c);
}

void syscall_type(ForthVM *vm) {
    // Syscall to print the string pointed to by the top of stack
    // The forst word of the string is the length
    // ( addr - )
    uint16_t dp = vm->pop();
    uint16_t a = vm->read(dp);
    uint16_t len = vm->read(a); // length in bytes
    a+=2;
    uint8_t b;
    for(uint16_t i=0; i<len; i++) {
        b = vm->readByte(a+i);
        printf("%c", b);    
    }
    fflush(stdout);
}

void syscall_dot(ForthVM *vm) {
    // Syscall to print the value on the top of the stack
    // This should respect the current BASE but that's for another day
    // ( v - )
    int base = vm->pop();
    int v = vm->pop();
    switch(base) {
        case 1:    
            printf("%04x", v);
            break;
        case 2:
            printf("%016b", v);
            break;
        case 0: 
        default:
            printf("%4d", v);
            break;
        
    }
}

void syscall_getc(ForthVM *vm) {
    int c = Serial::getc();
    vm->push(c);
}

void syscall_putc(ForthVM *vm) {
    int i = Serial::putc(vm->pop());
    vm->push(i);
}

void syscall_inline(ForthVM *vm) {
    // address of the buffer struct
    uint16_t buf = vm->pop();
    char *cbuf = vm->ram()->addressOfChar(buf+4);
    if(fgets(cbuf, 127, stdin) != NULL) {
        // There will now be a null-terminated string in the buffer
        // calculate the end and store that in buf+2
        vm->ram()->put(buf+2, buf + 4 + strlen(cbuf));
        // Current buffer pointer is just the start of the buffer
        vm->ram()->put(buf,buf+4);
    }
}

void syscall_flush(ForthVM *vm) {
    fflush(stdout);
}

/**
 * This class bootstraps the ForthVM to run the Ukmaker Forth-like language
*/
class TIL {

    public:

    TIL(ForthVM &vm) : _vm(vm) {

        _vm.addSyscall(SYSCALL_PRINTC, syscall_printC);
        _vm.addSyscall(SYSCALL_TYPE, syscall_type);
        _vm.addSyscall(SYSCALL_DOT, syscall_dot);
        _vm.addSyscall(SYSCALL_GETC, syscall_getc);
        _vm.addSyscall(SYSCALL_PUTC, syscall_putc);
        _vm.addSyscall(SYSCALL_INLINE, syscall_inline);
        _vm.addSyscall(SYSCALL_FLUSH, syscall_flush);

    }
    ~TIL() {}

    /**
     * Setup the environment
     * This should be called from the Arduino framework's setup() method
    */
    void boot() {
        // use the program counter to generate addresses
        _vm.reset();

        // Start the interpeter at address 50
        _vm.set(REG_PC, 50);

        _loadInnerInterpreter();
        _loadType();
        _loadTrap();
        _loadInline();
        _loadOuterInterpreter();

        // now we have the address of trap ready to use we can load the start/restart code
        _vm.set(REG_PC, 0);
        _loadStartRestart();
        _vm.reset();
        // And we're ready to start running
    }

    /**
     * Execute a VM instruction
     * This should be called from the Arduino framework's loop() method
    */
   void next() {
    _vm.step();
   }

   void run() {
    while(true) next();
   }


    protected:

    ForthVM &_vm;
    uint16_t _trapWA;
    uint16_t _ooWA;
    uint16_t _inlineWA;

    uint16_t _bufferAddr;
    uint16_t _bufferLen;
    uint16_t _bufferPtr;

    uint16_t _runAddr;
    uint16_t _nextAddr;
    uint16_t _colonAddr;
    uint16_t _semiAddr;
    uint16_t _dictionaryBase;
    uint16_t _current; // pointer to the top of the dictionary (i.e. the last defined word)

    void _loadStartRestart() {
        // setup the stacks
        _vm.load(0,0,OP_LDL, REG_SP,0); // teeny stacks for now
        _vm.load(1000);
        _vm.load(0,0,OP_LDL, REG_RS,0);
        _vm.load(1200);
        _vm.load(0,0,OP_LDL, REG_I, 0);
        _vm.load(_ooWA + 2);
        // and jump to NEXT
        _vm.load(0,0,OP_JP,0);
        _vm.load(_nextAddr);
    }

    void _loadInnerInterpreter() {
        // COLON
        _colonAddr = _vm.get(REG_PC);
        _vm.load(0,0,OP_PUSHR, REG_I,0);
        _vm.load(0,0,OP_LD, REG_I, REG_WA);
        _vm.load(0,0,OP_JR, 3); // Jup to NEXT
        // SEMI
        _semiAddr = _vm.get(REG_PC);
        _vm.load(_vm.get(REG_PC) + 2); // code address of semi
        _vm.load(0,0,OP_POPR, REG_I,0);
        // NEXT
        _nextAddr = _vm.get(REG_PC);
        _vm.load(0,0,OP_LDIX, REG_WA, REG_I); 
        _vm.load(0,0,OP_LDAI, 2); // next word
        _vm.load(0,0,OP_ADD,REG_I, REG_A);
        // RUN
        _runAddr = _vm.get(REG_PC);
        _vm.load(0,0,OP_LDIX, REG_CA, REG_WA);
        _vm.load(0,0,OP_ADD,REG_WA,REG_A);
        _vm.load(0,0,OP_LD,REG_PC,REG_CA);
    }

    void _loadType() {
        // this is a syscall up to printf
        // this is the first word defined in the dictionary
        _dictionaryBase = _vm.get(REG_PC);
        _current = 0;
        _startPrimitive("TYPE"); // no link address since this is the first word
        _vm.load(0,0,OP_SYSCALL, SYSCALL_TYPE);
        _endPrimitive();
    }

    void _loadTrap() {
        // Dump the contents of the registers
        // Say hello world first
        _trapWA = _startPrimitive("TRAP");
        _vm.load(0,0,OP_PUSHD, REG_PC, 0);
        _vm.load(0,0,OP_POPD, REG_A, 0);
        _vm.load(0,0,OP_LDBI, 8);
        _vm.load(0,0,OP_ADD, REG_A, REG_B);
        _vm.load(0,0, OP_JR, 8); // skip over the string
        _startWord("Hello World!\n");
        // code restarts here
        // push the address of the string
        _vm.load(0,0,OP_PUSHD, REG_A, 0);
        // And call type
        _vm.load(0,0,OP_SYSCALL, SYSCALL_TYPE);
        _endPrimitive();
    }

    void _loadInline() {
        _bufferAddr = 2000;
        _bufferLen = 4;
        _inlineWA = _startPrimitive("INLINE");
        
        // REG_I - length of the buffer
        // REG_A - scratchpad
        // REG_B - current buffer length
        // REG_WA - buffer pointer

        _vm.load(0,0,OP_PUSHD,REG_I,0);
        _vm.load(0,0,OP_PUSHD,REG_WA,0);

        // reset the pointer to the line buffer
        _vm.load(0,0,OP_LDL,REG_WA,0);
        _vm.load(_bufferAddr+2);

        // length of the buffer
        _vm.load(0,0,OP_LDL, REG_I,0);
        _vm.load(_bufferLen);

        _vm.load(0,0,OP_LD,REG_B,REG_I);

        // Loop to clear the buffer
        // CLB:
        _vm.load(0,0,OP_STI, REG_WA, 0);
        _vm.load(0,0,OP_ADDI, REG_WA, 1);
        _vm.load(0,0,OP_ADDBI, -1);
        _vm.load(COND_Z, COND_APPLY_NOT_MATCH, OP_JR, -4); // CLB:

         // reset the pointer to the line buffer
        _vm.load(0,0,OP_LDL,REG_WA,0);
        _vm.load(_bufferAddr+2);
        
        // now get input charcters to fill the buffer
        // we'll ignore backspace for now just to get started
        // input is terminated when a \n is seen
        // input longer than the buffer length is ignored
        // INCHAR:
        _vm.load(0,0,OP_SYSCALL, SYSCALL_GETC);
        _vm.load(0,0,OP_POPD, REG_A,0);

        _vm.load(0,0,OP_CMPAI, '\n');
        _vm.load(COND_Z,COND_APPLY_MATCH,OP_JR, 17); // END:

        _vm.load(0,0,OP_CMPAI, '\b');
        _vm.load(COND_Z, COND_APPLY_MATCH, OP_JR, 5);  // CHAR:
        // It's a backspace. Emit it and go for another char
        // Emit the backspace
        _vm.load(0,0,OP_PUSHD,REG_A,0);
        _vm.load(0,0,OP_SYSCALL, SYSCALL_PUTC);
        // Ignore the return value from PUTC for now
        _vm.load(0,0,OP_POPD,0,0);
        // decrement the buffer pointer if not at the beginning
        _vm.load(0,0,OP_CMPBI, 0);
        _vm.load(COND_Z, COND_APPLY_NOT_MATCH, OP_JR, -11);
        _vm.load(0,0,OP_ADDBI, -1);
        // go for another char
        _vm.load(0,0,OP_JR,-13); // INCHAR:

        // CHAR: append to the end of the buffer
        // provided we're not at the end already
        _vm.load(0,0,OP_CMP,REG_B,REG_I);
        _vm.load(COND_Z, COND_APPLY_MATCH, OP_JR,-15);

        _vm.load(0,0,OP_STAIXB, REG_WA, REG_B);
        _vm.load(0,0,OP_ADDBI, 1);

        // and back to INCHAR:
        _vm.load(0,0,OP_JR,-18);

        // LENGTH:
        // Write the length to the start of the buffer
        _vm.load(0,0,OP_LDL,REG_WA,0);
        _vm.load(_bufferAddr);
        _vm.load(0,0,OP_ST,REG_WA,REG_B);
        // END:
        _vm.load(0,0,OP_POPD,REG_WA,0);
        _vm.load(0,0,OP_POPD,REG_I,0);

        _endPrimitive();

    }

    void _loadOuterInterpreter() {
        // To start with this just calls trap
        _ooWA = _startSecondary("OO");
        _vm.load(_trapWA);
        _vm.load(_inlineWA);
        _endSecondary();
    }

    uint16_t _startPrimitive(const char *name) {
        uint16_t la = _current;
        _current = _vm.get(REG_PC);
        uint16_t ca = _startWord(name) + 4;
        _vm.load(la);
        uint16_t wa = _vm.get(REG_PC);
        _vm.load(ca);
        return wa;
    }

    void _endPrimitive() {
        _vm.load(0,0,OP_JP, 0);
        _vm.load(_runAddr);
    }

    uint16_t _startSecondary(const char *name) {
        uint16_t la = _current;
        _current = _vm.get(REG_PC);
        _startWord(name);
        _vm.load(la);
        uint16_t wa = _vm.get(REG_PC);
        _vm.load(_colonAddr);
        return wa;
    }

    void _endSecondary() {
        _vm.load(_semiAddr);
    }

    uint16_t _startWord(const char *name) {
        uint16_t len = 0;
        const char *p = name;
        while(*p != 0) {
            len++;
            p++;
        }
        _vm.load(len);
        return _vm.load(name);
    }

};
#endif