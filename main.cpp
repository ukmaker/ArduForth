
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include "runtime/ForthVM.h"
#include "runtime/UnsafeMemory.h"
#include "runtime/syscalls.h"
#include "tools/Assembler.h"
#include "tools/Dumper.h"
#include "tools/Debugger.h"
#include "tools/host_syscalls.h"

#include "tests/Test.h"
#include "tests/VMTests.h"
#include "tests/SlurpTests.h"
#include "tests/RangeTests.h"
#include "tests/LabelTests.h"
#include "tests/ConstantTests.h"
#include "tests/AliasTests.h"

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
ConstantTests constantTests(testSuite, &vm, &fasm);
AliasTests aliasTests(testSuite, &vm, &fasm);

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
  fasm.slurp("forth/core.fasm");
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
}

int main(int argc, char **argv)
{

  // testAssembler();
  // testRanges();
  // labelTests.run();
  // generateTestCode();
  // testVM();
  aliasTests.run();

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
    dumper.writeCPP(&fasm, &mem, 0, 8192);

    debugger.setAssembler(&fasm);
    debugger.setVM(&vm);
    debugger.reset();

    debugger.setBreakpoint1(0x0d2a);
   //debugger.setLabelBreakpoint2("TWO_DOTS_CA");
    // debugger.setLabelBreakpoint2("TOKEN_END");
    // ram.setWatch(0x5d4);
    //debugger.setShowForthWordsOnly();
   // debugger.setVerbose(true);
    // debugger.setBump(10);
    // debugger.writeProtect("DICTIONARY_END");
    debugger.run();

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