
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

#include "RAM.h"
#include "ForthVM.h"
#include "syscalls.h"
#include "Assembler.h"
#include "Dumper.h"
#include "Test.h"
#include "Debugger.h"

#include "VMTests.h"
#include "SlurpTests.h"
#include "RangeTests.h"
#include "LabelTests.h"


uint8_t memory[8192];

RAM ram(memory, 8192);

Syscall syscalls[40];

ForthVM vm(&ram, syscalls, 40);

Assembler fasm;
Dumper dumper;
Debugger debugger;

TestSuite *testSuite = new TestSuite();
VMTests vmTests(testSuite, &vm, &fasm);
RangeTests rangeTests(testSuite, &vm, &fasm);
LabelTests labelTests(testSuite, &vm, &fasm);
SlurpTests slurpTests(testSuite, &vm, &fasm);

int tests = 0;
int passed = 0;
int failed = 0;

bool runInstructionTests;
bool runAssemblerTests;
bool runGenerateTestCode;
bool verbose;

void syscall_debug(ForthVM *vm) {
    // WA points to the word to be executed
    // get the associated label and print it
    uint16_t wa = vm->get(REG_WA);
    //debugger.printWALabel(wa);
}


bool getArgs(int argc, char **argv) {

    int index;
    int c;

    opterr = 0;
    runInstructionTests = false;
    runAssemblerTests = false;
    runGenerateTestCode = false;
    verbose = false;

    while ((c = getopt (argc, argv, "iagv")) != -1) {
     switch (c)
       {
       case 'i':
         runInstructionTests = 1;
         break;
       case 'a':
         runAssemblerTests = 1;
         break;
       case 'g':
         runGenerateTestCode = 1;
         break;
        case 'v':
            verbose = true;
            break;
       case '?':
         if (isprint (optopt)) {
           fprintf (stderr, "Unknown option `-%c'.\n", optopt);
         } else {
           fprintf (stderr,
                    "Unknown option character `\\x%x'.\n",
                    optopt);
         }
         return false;
       default:
         abort ();
       }
    }

    return true;
}

bool loadInnerInterpreter() {
    fasm.slurp("forth/core.asm");
    fasm.pass1();
    fasm.pass2();
    fasm.pass3();

    fasm.dump();

    //if(verbose) 
    //
    //fasm.writeCode(); 
    return !fasm.hasErrors();  
}

void attachSyscalls() {
    vm.addSyscall(SYSCALL_PRINTC, syscall_printC);
    vm.addSyscall(SYSCALL_TYPE, syscall_type);
    vm.addSyscall(SYSCALL_TYPELN, syscall_typeln);
    vm.addSyscall(SYSCALL_DOT, syscall_dot);
    vm.addSyscall(SYSCALL_GETC, syscall_getc);
    vm.addSyscall(SYSCALL_PUTC, syscall_putc);
    vm.addSyscall(SYSCALL_INLINE, syscall_inline);
    vm.addSyscall(SYSCALL_FLUSH, syscall_flush);
    vm.addSyscall(SYSCALL_NUMBER, syscall_number);
    vm.addSyscall(SYSCALL_DEBUG, syscall_debug);
}

int main(int argc, char **argv) {
   //testAssembler();
    //testRanges();
    //labelTests.run();
    //generateTestCode();
    //testVM();

    /*
    if(getArgs(argc, argv)) {
        if(runAssemblerTests) testAssembler();
        if(runInstructionTests) testVM();
        if(runGenerateTestCode) generateTestCode();
    }
*/
    attachSyscalls();
    for(int i=0; i<16000; i++) 
    {
        Token *arse = new Token(NULL,0,0);
    }
    if(loadInnerInterpreter()) {
      dumper.dump(&fasm);
      dumper.writeCPP(&fasm);
      fasm.writeRAM(&ram);

      debugger.setAssembler(&fasm);
      debugger.setVM(&vm);
      debugger.reset();
      //debugger.setBreakpoint1(0x238);
      debugger.setLabelBreakpoint1("STAR_UNTIL_CA");
     // debugger.setLabelBreakpoint2("TOKEN_END");
      ram.setWatch(0x5d4);
      debugger.setShowForthWordsOnly();
     // debugger.setVerbose(true);
      //debugger.setBump(10);
      debugger.writeProtect("DICTIONARY_END");
      debugger.run();
    } else {
      printf("Assembly errors - exiting\n");
      printf("Phase 1 - %s\n", fasm.phase1Error ? "FAILED" : "OK");
      printf("Phase 2 - %s\n", fasm.phase2Error ? "FAILED" : "OK");
      printf("Phase 3 - %s\n", fasm.phase3Error ? "FAILED" : "OK");
    }

    

    return 0;
}