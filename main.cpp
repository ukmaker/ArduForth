
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include "runtime/ArduForth/ForthVM.h"
#include "runtime/ArduForth/UnsafeMemory.h"
#include "runtime/ArduForth/syscalls.h"
#include "tools/Assembler.h"
#include "tools/Loader.h"
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
Loader loader(&mem);

Assembler fasm;
Dumper dumper;
Debugger debugger;

TestSuite *testSuite = new TestSuite();
VMTests vmTests(testSuite, &vm, &fasm, &loader);
RangeTests rangeTests(testSuite, &vm, &fasm, &loader);
LabelTests labelTests(testSuite, &vm, &fasm, &loader);
SlurpTests slurpTests(testSuite, &vm, &fasm, &loader);

int tests = 0;
int passed = 0;
int failed = 0;

bool runInstructionTests;
bool runAssemblerTests;
bool verbose;

#define MODE_ATMEGA 0
#define MODE_STM32 1
int mode = MODE_ATMEGA;
bool loaded = false;

void syscall_debug(ForthVM *vm)
{
  // Provided as a convenience
  // Add your code as needed
}

void attachSyscalls(ForthVM *vm)
{
  vm->addSyscall(SYSCALL_DEBUG, syscall_debug);
  vm->addSyscall(SYSCALL_TYPE, syscall_type);
  vm->addSyscall(SYSCALL_TYPELN, syscall_typeln);
  vm->addSyscall(SYSCALL_DOT, syscall_dot);
  vm->addSyscall(SYSCALL_GETC, syscall_getc);
  vm->addSyscall(SYSCALL_PUTC, syscall_putc);
  vm->addSyscall(SYSCALL_INLINE, syscall_inline);
  vm->addSyscall(SYSCALL_FLUSH, syscall_flush);
  vm->addSyscall(SYSCALL_NUMBER, syscall_number);
  vm->addSyscall(SYSCALL_COMPARE, syscall_compare);

  /**
   * I can't see any reason to want to access the host desktop
   * Also, these routines would then need to be 64-bit
  */
  vm->addSyscall(SYSCALL_H_AT, syscall_unimplemented);
  vm->addSyscall(SYSCALL_H_STORE, syscall_unimplemented);
  vm->addSyscall(SYSCALL_ARDUINO, syscall_unimplemented);

  vm->addSyscall(SYSCALL_FREE_MEMORY, syscall_free_memory);
  vm->addSyscall(SYSCALL_SYSCALL, syscall_syscall);
  vm->addSyscall(SYSCALL_QEXIT, syscall_qexit);

  vm->addSyscall(SYSCALL_D_ADD, syscall_add_double);
  vm->addSyscall(SYSCALL_D_SUB, syscall_sub_double);
  vm->addSyscall(SYSCALL_D_DIV, syscall_mul_double);
  vm->addSyscall(SYSCALL_D_MUL, syscall_div_double);

  vm->addSyscall(SYSCALL_D_SR, syscall_sr_double);
  vm->addSyscall(SYSCALL_D_SL, syscall_sl_double);
  vm->addSyscall(SYSCALL_D_AND, syscall_and_double);
  vm->addSyscall(SYSCALL_D_OR, syscall_or_double);

  vm->addSyscall(SYSCALL_DOTC, syscall_dot_c);
  
  vm->addSyscall(SYSCALL_FOPEN, syscall_fopen);
  vm->addSyscall(SYSCALL_FCLOSE, syscall_fclose);
  vm->addSyscall(SYSCALL_FREAD, syscall_fread);
}

char getCommand() {
  int c = getchar();
  return (char)c;
}

void modeAtmega328() {
  fasm.clearOptions();
  fasm.setOption("#RAMSTART", 0x2400); 
  fasm.setOption("#VARSTART", 0x2600); 
  fasm.setOption("#SPTOP", 0x2780);    
  fasm.setOption("#RSTOP", 0x27fe);   
  fasm.setOption("#LBUF_LEN", 80); 
  mode = MODE_ATMEGA; 
  loaded = false;
}

void modeSTM32() {
  fasm.clearOptions();
  fasm.setOption("#RAMSTART", 0x2000); 
  fasm.setOption("#VARSTART", 0x3000); 
  fasm.setOption("#SPTOP", 0x3f80);    
  fasm.setOption("#RSTOP", 0x3ffe); 
  fasm.setOption("#LBUF_LEN", 128); 
  mode = MODE_STM32; 
  loaded = false;
}

void setMode() {
  if(mode == MODE_ATMEGA) 
  { 
    modeAtmega328();
  } else {
    modeSTM32();
  }
}

bool loadInnerInterpreter()
{

  setMode();
  fasm.slurp("fasm/core.fasm");
  fasm.pass1();
  fasm.pass2();
  fasm.pass3();

  if(!fasm.hasErrors()) 
  {
    // For development purposes we set the DP to just after the last
    // defined word sine everything is in RAM
    // When deployed to a board DP will point to the start of RAM
    fasm.writeMemory(&mem);
    Symbol *sym = fasm.getSymbol("DICTIONARY_END");
    vm.ram()->put(10, sym->token->address);
    loaded = true;
  }

  return !fasm.hasErrors();
}

void generateCPP() {
  if(loaded) 
  {
    uint16_t romsize = vm.read(10);

    Symbol *sym = fasm.getSymbol("#RAMSTART");
    vm.ram()->put(10, sym->token->value);

    switch(mode) {
      case MODE_ATMEGA:
        dumper.writeCPP("ForthImage_ATMEGA328.h", &fasm, &mem, 0, romsize, true);
        break;

      case MODE_STM32:
        dumper.writeCPP("ForthImage_STM32.h", &fasm, &mem, 0, romsize, true); 
        break;

      default: break;
    }

    sym = fasm.getSymbol("DICTIONARY_END");
    vm.ram()->put(10, sym->token->address);

  }
  else
  {
    Serial.println("Image not loaded. Did you change the target?");
  }
}

void prompt() {
  if(mode == MODE_ATMEGA) 
  { 
    Serial.print("ATMEGA >>"); 
  } 
  else 
  { 
    Serial.print("STM32 >>"); 
  }
}

bool commandLine() {
  char c;
  while((char)(c = getCommand()) == '\n') {
    prompt();
  }
  switch(c) {
    case 'A': modeAtmega328(); break;
    case 'G': generateCPP(); break;
    case 'S': modeSTM32(); break; 
    case 'c': vm.reset(); vm.run(); prompt(); break;
    case 'w': vm.warm();  vm.run(); prompt(); break;
    case 'l': loadInnerInterpreter(); break;
    case 'e': return false;
    case 'd': debugger.run(); prompt(); break;
    case '?': 
      Serial.println("Commands:");
      Serial.println("A - Set mode ATMEGA328");
      Serial.println("S - Set mode STM32");
      Serial.println("G - Generate ForthImage_xxx.h");
      Serial.println("l - (Re)Load the minimal image");
      Serial.println("c - Cold start the Forth VM");
      Serial.println("w - Warm start the Forth VM");
      Serial.println("d - Run the debugger");
      Serial.println("e - Exit");
      break;
    default: break;
  }
  return true;
}

int main(int argc, char **argv)
{

  attachSyscalls(&vm);

  vmTests.run();
  rangeTests.run();
  labelTests.run();
  slurpTests.run();

  if (loadInnerInterpreter())
  {

    debugger.setAssembler(&fasm);
    debugger.setVM(&vm);
    debugger.reset();
    debugger.setBreakpoint1(0x1110);
    debugger.setLabelBreakpoint1("SEARCH_CA");
    debugger.setLabelBreakpoint2("SEARCH_CA");
    // ram.setWatch(0x5d4);
    debugger.setShowForthWordsOnly();
    debugger.setVerbose(true);
    // debugger.setBump(10);
    // debugger.writeProtect("DICTIONARY_END");
    //debugger.run();
    prompt();
    while(commandLine()) ;
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