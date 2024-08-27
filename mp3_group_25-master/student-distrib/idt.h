#ifndef IDT_H
#define IDT_H

#include "types.h"

extern void initialize_idt(void);

extern void exception_handler(uint32_t offset);

#endif
