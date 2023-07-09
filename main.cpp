
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include "runtime/ArduForth/ForthVM.h"
#include "runtime/ArduForth/UnsafeMemory.h"
#include "runtime/ArduForth/syscalls.h"
#include "tools/Assembler.h"
#include "tools/Dumper.h"
#include "tools/Debugger.h"
#include "tools/host_syscalls.h"

#include "tests/Test.h"
#include "tests/VMTests.h"
#include "tests/SlurpTests.h"
#include "tests/RangeTests.h"
#include "tests/LabelTests.h"

#define GENERATE_328P

/*
* core.asm defines
*
* RAMSTART 0x1000
* RAMSIZE  0x3000
*
* So here we define RAM to be sized 0x4000 to hold everything
*/
uint8_t ram[16384];

uint8_t rom[32];

UnsafeMemory mem(ram, 16384, 0, rom, 64, 16384);

Syscall syscalls[40];

ForthVM vm(&mem, syscalls, 40);

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

void syscall_debug(ForthVM *vm)
{
  // WA points to the word to be executed
  // get the associated label and print it
  uint16_t wa = vm->get(REG_WA);
  // debugger.printWALabel(wa);
}


void syscall_write_cpp(ForthVM *vm) {
  dumper.writeCPP("ForthImage.h", &fasm, &mem, 0, 8192);
}

bool getArgs(int argc, char **argv)
{

  int index;
  int c;

  opterr = 0;
  runInstructionTests = false;
  runAssemblerTests = false;
  runGenerateTestCode = false;
  verbose = false;

  while ((c = getopt(argc, argv, "iagv")) != -1)
  {
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
      if (isprint(optopt))
      {
        fprintf(stderr, "Unknown option `-%c'.\n", optopt);
      }
      else
      {
        fprintf(stderr,
                "Unknown option character `\\x%x'.\n",
                optopt);
      }
      return false;
    default:
      abort();
    }
  }

  return true;
}

bool loadInnerInterpreter()
{

#ifdef GENERATE_328P
  fasm.setOption("#RAMSTART", 0x2000); //  #RAMSTART: 0x2000  ; Need to allocate 8K for Forth ROM
  fasm.setOption("#VARSTART", 0x2400); //  #VARSTART: 0x2400  ; Allow 1K for new Forth words in RAM. Allot 256 bytes for variables
  fasm.setOption("#SPTOP", 0x2520);    //  #SPTOP:    0x2520  ; Vars end at 0x2500 - 32 bytes for the data stack
  fasm.setOption("#RSTOP", 0x2540);    //  #RSTOP:    0x2540  ; 32 bytes for the return stack leaves 0x2C0 Bytes of RAM free for Arduino#endif
#endif

  fasm.slurp("fasm/core.fasm");
  fasm.pass1();
  fasm.pass2();
  fasm.pass3();

  fasm.dump();

  // if(verbose)
  //
  // fasm.writeCode();
  return !fasm.hasErrors();
}

void attachSyscalls()
{
  vm.addSyscall(SYSCALL_DEBUG, syscall_debug);
  vm.addSyscall(SYSCALL_TYPE, syscall_type);
  vm.addSyscall(SYSCALL_TYPELN, syscall_typeln);
  vm.addSyscall(SYSCALL_DOT, syscall_dot);
  vm.addSyscall(SYSCALL_GETC, syscall_getc);
  vm.addSyscall(SYSCALL_PUTC, syscall_putc);
  vm.addSyscall(SYSCALL_INLINE, syscall_inline);
  vm.addSyscall(SYSCALL_FLUSH, syscall_flush);
  vm.addSyscall(SYSCALL_NUMBER, syscall_number);
  vm.addSyscall(SYSCALL_H_AT, syscall_read_host);
  vm.addSyscall(SYSCALL_H_STORE, syscall_write_host);
  vm.addSyscall(SYSCALL_D_ADD, syscall_add_double);
  vm.addSyscall(SYSCALL_D_SUB, syscall_sub_double);
  vm.addSyscall(SYSCALL_D_DIV, syscall_mul_double);
  vm.addSyscall(SYSCALL_D_MUL, syscall_div_double);

  vm.addSyscall(SYSCALL_D_SR, syscall_sr_double);
  vm.addSyscall(SYSCALL_D_SL, syscall_sl_double);
  vm.addSyscall(SYSCALL_D_AND, syscall_and_double);
  vm.addSyscall(SYSCALL_D_OR, syscall_or_double);

  vm.addSyscall(SYSCALL_DOTC, syscall_dot_c);
  
  vm.addSyscall(SYSCALL_FOPEN, syscall_fopen);
  vm.addSyscall(SYSCALL_FCLOSE, syscall_fclose);
  vm.addSyscall(SYSCALL_FREAD, syscall_fread);
  vm.addSyscall(SYSCALL_WRITE_CPP, syscall_write_cpp);
}

int main(int argc, char **argv)
{

  // testAssembler();
  // testRanges();
  // labelTests.run();
  // generateTestCode();
  // testVM();

  /*
  if(getArgs(argc, argv)) {
      if(runAssemblerTests) testAssembler();
      if(runInstructionTests) testVM();
      if(runGenerateTestCode) generateTestCode();
  }
*/
  attachSyscalls();

  if (loadInnerInterpreter())
  {
    dumper.dump(&fasm);
    fasm.writeMemory(&mem);
    #ifdef GENERATE_328P
    dumper.writeCPP("ForthImage_ATMEGA328.h", &fasm, &mem, 0, 8192);
    #else
    dumper.writeCPP("ForthImage_STM32F4xx.h", &fasm, &mem, 0, 8192);
    #endif

    debugger.setAssembler(&fasm);
    debugger.setVM(&vm);
    debugger.reset();
    debugger.setBreakpoint1(0x1110);
    // debugger.setLabelBreakpoint1("STAR_UNTIL_CA");
    // debugger.setLabelBreakpoint2("TOKEN_END");
    // ram.setWatch(0x5d4);
    // debugger.setShowForthWordsOnly();
    // debugger.setVerbose(true);
    // debugger.setBump(10);
    // debugger.writeProtect("DICTIONARY_END");
    //debugger.run();

    vm.run();
  }
  else
  {
    printf("Assembly errors - exiting\n");
    printf("Phase 1 - %s\n", fasm.phase1Error ? "FAILED" : "OK");
    printf("Phase 2 - %s\n", fasm.phase2Error ? "FAILED" : "OK");
    printf("Phase 3 - %s\n", fasm.phase3Error ? "FAILED" : "OK");
  }

  return 0;
}