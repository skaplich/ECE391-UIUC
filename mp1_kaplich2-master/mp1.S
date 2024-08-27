

.data                                   # section declaration


        # Useful offset constants for accessing members of a
        # struct mp1_blink_struct structure
        LOCATION   = 0    
        ON_CHAR    = 2
        OFF_CHAR   = 3
        ON_LENGTH  = 4
        OFF_LENGTH = 6
        COUNTDOWN  = 8
        STATUS     = 10
        NEXT       = 12


        STRUCT_SIZE = 16


# Pointer to head of list (initialized to NULL)
mp1_list_head:
        .long   0


.text                                   # section declaration


# Export the function symbol names


.global mp1_rtc_tasklet
.global mp1_ioctl


# void mp1_poke(void);
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX
mp1_poke:
       
        movl    vmem_base_addr(,1),%edx
        movb    %cl,(%edx,%eax,1)
        ret

# void mp1 rtc tasklet (unsigned long);
#
# Interface: Register-based arguments (not C-style)
#    Inputs: mp1_list_head (variable stored outside of function)
#   Outputs: Updates the state of the game
# Registers: Clobbers EAX, ECX
mp1_rtc_tasklet:
        pushl %ebp      # building the stack frame
        movl %esp, %ebp
        pushl %esi      # push old value of ESI (source index register)
        pushl %edi      # push old value of EDI (Destination index register)
        pushl %ebx
                   
        # set esi to the original head          
        movl mp1_list_head, %esi
       
        # check if we are at the end of the linked list
        tasklet_end:
        cmpl $0, %esi
        je tasklet_teardown
        add $-1, COUNTDOWN(%esi)
        cmpw $0, COUNTDOWN(%esi)
        je tasklet_examine_status
       
        tasklet_traverse_list:
        movl NEXT(%esi), %esi
        jmp tasklet_end


        tasklet_examine_status:
        cmpw $1, STATUS(%esi)  # value is one so we need to turn off
        je turn_off_char
        cmp $0, STATUS(%esi)  # value is zero so we need to turn on
        je turn_on_char


        turn_on_char:
        movzwl LOCATION(%esi), %eax  
        movb ON_CHAR(%esi), %cl # onchar -> cl register
        imul $2, %eax
        call mp1_poke
        movw $1, STATUS(%esi) # update status
        movw ON_LENGTH(%esi), %dx  # onlength -> countdown
        movw %dx, COUNTDOWN(%esi)
        jmp tasklet_traverse_list


        turn_off_char:
        movzwl LOCATION(%esi), %eax  
        movb OFF_CHAR(%esi), %cl # offchar -> cl register      
        imul $2, %eax
        call mp1_poke
        movw $0, STATUS(%esi) # update status
        movw OFF_LENGTH(%esi), %dx  # onlength -> countdown
        movw %dx, COUNTDOWN(%esi)
        jmp tasklet_traverse_list


        tasklet_teardown:
        pop %ebx
        pop %edi
        pop %esi
        leave
        ret


# int mp1_ioctl (unsigned long arg, unsigned long cmd);
#
# Interface: Register-based arguments (not C-style)
#    Inputs: 8(%esp) in memory contains cmd value
#               4(%esp) contains the argument to call a helper function
#   Outputs: Controller that calls the core functions based on the cmd value
#               -1 is unsuccessful execution
# Registers: Clobbers EAX, EBX
mp1_ioctl:
        movl 8(%esp),%eax   # eax register contains cmd value
        movl 4(%esp),%ebx   # ebx contains the argument
       
        # checking for invalid input
        cmp $0, %eax    
        jl invalid_input
        cmp $3, %eax
        jg invalid_input


        # check for correct input        
        cmp $0, %eax
        je valid_input
        cmp $1, %eax
        je valid_input
        cmp $2, %eax
        je valid_input
        cmp $3, %eax
        je valid_input
        jmp invalid_input # did not pass check, input must be invalid


        valid_input:
        jmp *jump_table(,%eax,4)


        invalid_input:  
        movl $-1,%eax  
        ret

# int mp1 ioctl add(unsigned long arg)
#
# Interface: Register-based arguments (not C-style)
#    Inputs: 8(%ebp) in memory contains a pointer to a mp1_blink_structure
#              
#   Outputs: adds a struct to the linked list
#               0 = success
#               -1 = runtime error for the function
# Registers: Clobbers EBX
mp1_ioctl_add:
        # building the stack frame
        pushl %ebp      
        movl %esp, %ebp
        pushl %esi      # ESI (source index register)
        pushl %edi      # EDI (Destination index register)
        pushl %ebx      # EBX (god did)
        pushl %edx      # EDX
       
        movl 8(%ebp), %edx      # edx contains our original struct
        cmpl $0, %edx         # check if input pointer is null  
        je add_error_wrong_value


        # allocating memory for the new structure
        push %edx               # I figured out mp1_malloc clobbers %edx    BRUH MOMENT
        push $STRUCT_SIZE
        call mp1_malloc
        add $4, %esp
        pop %edx
        cmpl $0, %eax
        je add_error_wrong_value
        movl %eax, %esi         # esi now points to the empty allocated struct


        # copying data into struct
        pushl $STRUCT_SIZE    
        pushl %edx
        pushl %esi
        call mp1_copy_from_user # edx contains full copy
        pop %esi
        pop %edx
        add $4, %esp
       
        # checking if the copying worked
        cmp $0, %eax            
        je add_input_range
        jmp add_memory_error


        add_input_range:
        cmpw $1999, LOCATION(%esi)
        ja add_memory_error
        cmpw $0, LOCATION(%esi)    
        jb add_memory_error
       
        # we have passed all the checks, now edit the struct
        movl mp1_list_head, %eax          
        movl %eax, NEXT(%esi)
        movl %esi, mp1_list_head


        movw $1, STATUS(%esi)                   # set the status field to 1
        movw ON_LENGTH(%esi), %ax              
        movw %ax, COUNTDOWN(%esi)               # set countdown field equal to on_length field
        movzwl LOCATION(%esi), %eax             # location offset = eax      
        movb ON_CHAR(%esi), %cl                 # cl register = ONCHAR byte
        imul $2, %eax
        pushl %esi  
        pushl %edx                            # save esi just in case
        call mp1_poke
        pop %edx
        pop %esi
        nop
        jmp add_success


        add_memory_error:        
        push %esi
        call mp1_free
        pop %esi


        add_error_wrong_value:
        movl $-1, %eax  
        pop %edx
        add $12, %esp
        leave
        ret


        add_success:
        movl $0, %eax
        pop %edx
        add $12, %esp
        leave
        ret


        add_teardown:
        pop %edx
        add $12, %esp
        leave
        ret
       

# int mp1 ioctl remove(unsigned long arg)
#
# Interface: Register-based arguments (not C-style)
#    Inputs: 8(%ebp) takes an integer location as its parameter
#              
#   Outputs: removes an element from the linked list
#               0 = success
#               -1 = error or could not find element
# Registers: Clobbers EAX, EBX       
mp1_ioctl_remove:
        # take int location as parameter
        # traverse list to find location
        # remove that thing from the linked list
        # free its memory
        # teardown
        pushl %ebp      
        movl %esp, %ebp
        pushl %esi      # push old value of ESI (source index register)
        pushl %edi      # push old value of EDI (Destination index register)
        pushl %ebx
       
        movl 8(%ebp), %edi    # %edi contains the location we want to remove
        cmpl $0, %edi         # check if location is not zero
        je remove_error
 
        # find the struct on the list
        pushl %edi
        call search_helper
        pop %edi
        # check that it exists
        cmpl $-1, %eax
        je remove_error
        movl %eax, %ebx  # actual struct -> ebx


        # find previous node
        nop
        pushl %ebx            
        call modified_search_helper
        pop %ebx
        # check that previous node exists
        cmpl $-1, %eax
        je remove_error
        movl %eax, %ecx
       
        # remove node
        movl NEXT(%ebx), %esi  
        cmpl $0, %ecx          
        je skip_because_at_head
        movl %esi, NEXT(%ecx)    
        jmp remove_free
        skip_because_at_head:                
        movl %esi, mp1_list_head


        remove_free:            
        pushl %ebx
        call mp1_free
        pop %esp
        jmp remove_teardown


        remove_error:
        movl $-1, %eax
        pop %ebx
        pop %edi
        pop %esi
        leave
        ret

        remove_success:
        movl $0, %eax
        pop %ebx
        pop %edi
        pop %esi
        leave
        ret

        remove_teardown:
        pop %ebx
        pop %edi
        pop %esi
        leave
        ret
       
# int mp1 ioctl find(unsigned long arg)
#
# Interface: Register-based arguments (not C-style)
#    Inputs: 8(%ebp) takes an integer location as its parameter
#              
#   Outputs: Validates that an element exists in the linked list
#               0 = success
#               -1 = error or could not find element
# Registers: Clobbers EAX 
mp1_ioctl_find:
        # build the stack frame (ESI, EDI, EBX)
        # move initial struct pointer into esi
        # check if that esi pointer is valid
        # allocate the memory needed to store the input structure
        # check if this malloc was successful
        # copy this structure from the user space to the kernel
        # write the copy to the register edi
        # check if register copy worked
        # check the bounds of the copy located on the kernel
        # save the LOCATION variable of the copy to some register
        # search mp1_list_head for an element with the same location
        # do this by invoking a helper function that receives a location
        # check if the helper pointer exists/is valid
        # copy the output pointer to the original pointer in esi
        # tear the stack down


        # building the stack frame
        pushl %ebp      
        movl %esp, %ebp
        pushl %esi      # push old value of ESI (source index register)
        pushl %edi      # push old value of EDI (Destination index register)
        pushl %ebx


        # initial struct pointer
        movl 8(%ebp), %esi   # struct pointer -> %esi
        cmp $0, %esi
        je find_error                # return -1 and tear


        # allocate memory
        pushl %esi
        pushl $STRUCT_SIZE
        call mp1_malloc
        add $4, %esp
        pop %esi
        cmpl $0, %eax    # check if the malloc was successful
        je find_error
        movl %eax, %edi        


        # copy struct from user space to kernel
        pushl $STRUCT_SIZE      
        pushl %esi
        pushl %edi
        call mp1_copy_from_user
        pop %edi
        pop %esi
        add $4, %esp


        # check to see if the copy worked
        cmpl $0, %eax  
        je find_input_range
        jmp find_error_with_dealloc          # return -1, deallocate, tear
       
        # at this point in the code :
                # edi = kernel struct
                # esi = original usr pointer
               
        find_input_range:
        cmpw $1999, LOCATION(%edi)   # 0 < %edi < 80*25 - 1
        ja find_error_with_dealloc                    
        cmpw $0, LOCATION(%edi)
        jb find_error_with_dealloc


        # call helper function to find the struct
        pushw LOCATION(%edi)
        call search_helper
        addl $2, %esp
        cmpl $-1, %eax
        je find_error_with_dealloc


        # helper returned something, copy it back to user
        pushl $STRUCT_SIZE  
        pushl %eax
        pushl 8(%ebp)
        call mp1_copy_to_user
        add $12, %esp
       
        # check if copying back to user worked
        cmp $0, %eax      
        je find_success


        find_error:
        movl $-1, %eax
        jmp find_teardown


        find_error_with_dealloc:
        movl $-1, %eax
        pushl %edi
        call mp1_free
        pop %edi
        jmp find_teardown


        find_success:
        movl $0, %eax


        find_teardown:        
        pop %ebx
        pop %edi
        pop %esi
        leave
        ret



# int mp1 ioctl sync(unsigned long arg)
#
# Interface: Register-based arguments (not C-style)
#    Inputs: 8(%ebp) two-byte unsigned short integers, packed into one
# four-byte argument, upper 16 bits is int1, lower 16 bits is int 2
#              
#   Outputs: synchronizes two existing locations on the screen to have the same framerate
#               0 = success
#               -1 = error 
# Registers: Clobbers all General Registers 
mp1_ioctl_sync:
        # extract data from arg, store in two registers
                # location of first integer (upper) is the first char
                # location of second integer (lower) is the second char
        # read the first int, find it in the list, store it in a register
        # read the second int, find it in the list, store it in a register
        # copy timing info from first to second
        # call mp1_poke depending on status of first int
        # return appropriate values
        # tear stack
        pushl %ebp        
        movl %esp, %ebp
        pushl %esi
        pushl %edi
        pushl %ebx


        # move the integers into ebx
        movl 8(%ebp), %ebx  
        cmpl $0, %ebx
        je sync_error


        # first integer -> %edi
        movl %ebx, %edi
        shrl $16, %edi


        pushl %edi
        call search_helper
        pop %edi
        cmpl $-1, %eax          # check if location was found
        je sync_error
        movl %eax, %edi
       
        # second integer -> %edx
        xorl %edx, %edx
        movw %bx, %dx              


        pushl %edx
        call search_helper  
        pop %edx
        cmpl $-1, %eax          # check if location was found
        je sync_error
        movl %eax, %edx


        # copy timing info from the first to the second
        movw ON_LENGTH(%edi), %ax
        movw %ax, ON_LENGTH(%edx)


        movw OFF_LENGTH(%edi), %ax
        movw %ax, OFF_LENGTH(%edx)


        movw COUNTDOWN(%edi), %ax
        movw %ax, COUNTDOWN(%edx)


        movw STATUS(%edi), %ax
        movw %ax, STATUS(%edx)


        # check what the status is for first int
        cmpw $1, STATUS(%edx)
        je sync_on


        sync_off:  
        movzwl LOCATION(%edx), %eax  
        imul $2, %eax
        movb OFF_CHAR(%edx), %cl
        pushl %edx
        call mp1_poke
        pop %edx        
        jmp sync_success


        sync_on:
        movzwl LOCATION(%edx), %eax
        imul $2, %eax
        movb ON_CHAR(%edx), %cl
        pushl %edx
        call mp1_poke
        pop %edx
        jmp sync_success


        sync_error:
        movl $-1, %eax
        jmp sync_teardown


        sync_success:
        movl $0, %eax
        jmp sync_teardown


        sync_teardown:        
        popl %ebx      
        popl %edi
        popl %esi
        leave
        ret




#    Inputs: 8(%ebp) 32 bit LOCATION element of a of a mp1_struct
#              
#   Outputs: Returns the location of an element in the linked in %eax register
#               -1 = error 
# Registers: Clobbers EAX
search_helper:
        pushl %ebp          
        movl %esp, %ebp
        pushl %esi
        pushl %edi
        pushl %ebx


        movl 8(%ebp), %eax          
        movl mp1_list_head, %esi  
       
        search_traverse:      
        cmpl $0, %esi              
        je search_error
        cmpw %ax, LOCATION(%esi)
        je search_found_location
        movl NEXT(%esi), %esi
        jmp search_traverse


        search_found_location:      
        movl %esi, %eax
        jmp search_teardown     # good output : location of the struct


        search_error:        
        movl $-1, %eax          # -1 is an error for this helper
        jmp search_teardown


        search_teardown:
        popl %ebx        
        popl %edi
        popl %esi
        leave
        ret

# Very similar to the search_helper function
#    Inputs: 8(%ebp) 32 bit LOCATION element of a of a mp1_struct
#              
#   Outputs: Returns the location of the element in the linked list
#   that POINTS to the input location 
#               -1 = error 
# Registers: Clobbers EAX
modified_search_helper:
        pushl %ebp          
        movl %esp, %ebp
        pushl %esi
        pushl %edi
        pushl %ebx


        movl 8(%ebp), %ebx              # location -> %eax      
        movl mp1_list_head, %eax        # first item
        cmpl %eax, %ebx                 # checking if there is one item in the list
        je m_search_found_location      


        m_search_traverse:      
        cmpl $0, %ebx           # check if at end    !!!            
        je m_search_error       #                       !!!
        cmpl NEXT(%eax), %ebx
        je m_search_found_location
        movl NEXT(%eax), %eax
        jmp m_search_traverse


        m_search_found_location:      
        movl $0, %eax
        jmp m_search_teardown     # good output : location of the struct that points to the location passed in


        m_search_error:        
        movl $-1, %eax          # -1 is an error for this helper
        jmp m_search_teardown


        m_search_teardown:
        popl %ebx        
        popl %edi
        popl %esi
        leave
        ret
       

# jump table for the ioctl controller for calling core functions
jump_table:
        .long mp1_ioctl_add, mp1_ioctl_remove, mp1_ioctl_find, mp1_ioctl_sync


.end



# 2 truths, 1 lie:

# mp1 was a piece of cake
# x86 assembly is annoying
# sasha kaplich is good at coding
