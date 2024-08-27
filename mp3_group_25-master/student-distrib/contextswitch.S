#define ASM

#include "contextswitch.h"

.globl setUpStack

setUpStack:
movl 4(%esp),%edx
 movw $0x2B, %bx  
    movw %bx, %ds 

    pushl $0x02b  
    pushl $0x083ffffc 
    pushl $0x0206
    pushl $0x023 
    pushl %edx      
    iret          
    