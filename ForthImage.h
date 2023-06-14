#ifndef UKMAKER_FORTH_IMAGE_H
#define UKMAKER_FORTH_IMAGE_H

// Include configuration
#include "ForthConfiguration.h"

#ifndef RAM_SIZE
#define RAM_SIZE 1024 // Assign 1K to variables
#endif

#ifndef ROM_SIZE
#define ROM_SIZE 2048 // Assign 2K to core vocabulary
#endif


/**
 * Constants
*/
#define SOME_CONSTANT 0x1020

uint16_t ram[RAM_SIZE];

static const uint16_t rom[ROM_SIZE] = {

};