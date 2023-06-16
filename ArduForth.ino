/**
* Run a ForthVM image previously created using the Assembler
**/

#include <Arduino.h>
#include "runtime/ForthConfiguration.h"
#include "runtime/ForthVM.h"
#include "runtime/UnsafeMemory.h"
#include "runtime/syscalls.h"

#include "runtime/ForthImage.h"


uint8_t ram[16384];

UnsafeMemory mem(ram, 16384, 0x1000, rom, 4096, 0);

Syscall syscalls[40];

ForthVM vm(&mem, syscalls, 40);

void syscall_debug(ForthVM *vm)
{
  // WA points to the word to be executed
  // get the associated label and print it
  uint16_t wa = vm->get(REG_WA);
  // debugger.printWALabel(wa);
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

}


//int main(int argc, char **argv)

void setup() {

    SerialUSB.begin(9600);
    SerialUSB.printf("Starting Forth..\n");
    SerialUSB.flush();
    attachSyscalls();
    vm.reset();

}
void loop()
{

    vm.run();
    //return 0;
}