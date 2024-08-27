#ifndef TERMINALDRIVER_H
#define TERMINALDRIVER_H

//including these files
#include "types.h"
#include "lib.h"
#include "i8259.h"

//functions for terminal driver
extern int32_t closeterminal();
extern int32_t openterminal();
extern int32_t readterminal(uint32_t fd, uint8_t* buffer, uint32_t n);
extern int32_t writeterminal(uint32_t fd, const void* buffer, uint32_t n);


#endif 
