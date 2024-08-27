#define ASM

#include "IntHandler.h"

# keyboard wrapper that pushes registers and flags before calling keyboard handler
# inputs: none
# outputs: none
# actions: make sure EIP is restored so multiple interrupts can happen
.globl keyboardWrap, rtcWrap,pitWrap, pagingInit
keyboardWrap:
    
    pushal
    pushfl
    call handler_keyboard
    popfl
    popal
    iret

# rtc wrapper that pushes registers and flags before calling rtc handler
# inputs: none
# outputs: none
# actions: make sure EIP is restored so multiple interrupts can happen
rtcWrap:
    
    pushal
    pushfl
    call handler_rtc
    popfl
    popal
    iret

#linkage from pit due to it being an entry in IDT

pitWrap:
    
    pushal
    pushfl
    call handlepit
    popfl
    popal
    iret

#specify base of page table directory
#configure other register settings
pagingInit:
    pushl %ebp
    movl %esp, %ebp
    pushl %edi
    pushl %esi
    pushl %ebx
   

    movl 8(%ebp), %edi
    movl %edi, %cr3

    movl %cr4, %edi
    or $0x00000010, %edi
    movl %edi, %cr4

    movl %cr0, %edi
    or $0x80000001, %edi
    movl %edi, %cr0

    
    popl %ebx
    popl %esi
    popl %edi
    leave
    ret
    
