#define ASM
#include "syscall_handler.h"


.globl syscall_handler, halt_handler 
syscall_handler:
    pushl %esp
    pushl %ebp
    pushl %edi
    pushl %esi
    pushfl
    pushl %edx
    pushl %ecx
    pushl %ebx


    pushl %edx
    pushl %ecx
    pushl %ebx
    
    jmp compare

compare:
    cmpl $0, %eax
    jle invalid

    cmpl $10, %eax
    jg invalid

    call *jmp_table(,%eax,4)
    jmp done

done:
    
    addl $12, %esp
    popl %ebx
    popl %ecx
    popl %edx
    popfl
    popl %esi
    popl %edi
    popl %ebp
    popl %esp
    iret

invalid:
    
    movl $-1, %eax
    addl $12, %esp
    popl %ebx
    popl %ecx
    popl %edx
    popfl
    popl %esi
    popl %edi
    popl %ebp
    popl %esp
    iret


halt_handler:
    pushl %ebp
    movl %esp, %ebp

    movl $0, %ecx

    movl 16(%ebp), %ecx
    movl %ecx, %eax

    movl 12(%ebp), %ecx
    movl %ecx, %esp

    movl 8(%ebp), %ecx
    movl %ecx, %ebp

    leave
    ret

jmp_table:

    .long 0x00000000, halt, execute, read, write, open, close, getargs, vidmap 
    
