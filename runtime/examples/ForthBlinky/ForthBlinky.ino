#ifdef ARDUINO
/**
* Run a ForthVM image previously created using the Assembler
**/
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <Arduino.h>
#include "ForthVM.h"
#include "UnsafeMemory.h"
#include "syscalls.h"

#include "ForthImage_Generic.h"


uint8_t ram[16384];

UnsafeMemory mem(ram, 16384, 0x2000, rom, 8192, 0);

Syscall syscalls[40];

ForthVM vm(&mem, syscalls, 40);

void syscall_debug(ForthVM *vm)
{

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
  vm.addSyscall(SYSCALL_FREE_MEMORY, syscall_free_memory);
  vm.addSyscall(SYSCALL_SYSCALL, syscall_syscall);
  vm.addSyscall(SYSCALL_ARDUINO, syscall_arduino);
  }


void setup()
{
    pinMode(PC13, OUTPUT);
    digitalWrite(PC13, 1);
    attachSyscalls();
    Serial.begin(115200);
    vm.reset();
}

void loop() {
    for(int i=0; i<100; i++) vm.step();
}
#endif