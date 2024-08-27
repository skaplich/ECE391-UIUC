#define ASM
#include "invalidOpcode.h"


.globl opcodeTest, segmentErrorCall

opcodeTest:
    # Example of an invalid opcode
    ud2

segmentErrorCall:
   
    INT $0x80   
    
             