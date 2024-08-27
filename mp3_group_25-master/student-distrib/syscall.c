#include "syscall.h"
#include "x86_desc.h"
#include "contextswitch.h"
#include "terminaldriver.h"
#include "syscall_handler.h"
#include "scheduling.h"


int process_list[MAX_NUM_PROCESSES] = {-1,-1,-1,-1,-1,-1};   //only 3 max processes in this OS
int current_pid = -1;

/* 
 * halt
 *   DESCRIPTION: Halts the currently executing process.
 *   INPUTS: status - exit status of the process
 *   OUTPUTS: none
 *   RETURN VALUE: This function should not return
 *   SIDE EFFECTS: Exits the current process
 */
int32_t halt(uint8_t status) {
    // Function definition for halt
    // Function definition for halt
    cli();
    int32_t newStatus = status;
    if(newStatus == 255){       // you have to pass in 256 becase the argument is of type uint 8, so we add this edgecase
        newStatus = 256;
    }


    
    //get information about parent process
    pcb_t * parent_pcb = (pcb_t *)get_pcb_using_pid(current_pid);
    int i;
    for(i = 2; i < 8; i++){
        parent_pcb->fda[i].flags = 0;
    }
    int parent_pid = parent_pcb->parent_pid;
    
    
    if(terminal_info[scheduling_num].Pcount == 1){
        terminal_info[scheduling_num].Pcount--;
        process_list[current_pid] = -1;
        current_pid = parent_pid;
        sti();
        execute((uint8_t*)"shell");
    }

    terminal_info[scheduling_num].pid = parent_pid;

    //switch back to parent page
    set_process_memory(parent_pid);
	flush_tlb();

    //change tss to previous kernel space for corresponding to parent process
    tss.esp0 = MB_8 - parent_pid * (1024 * 8) - 4; //1kiB * 8 = 8kiB and 4 for the offset
    tss.ss0 = KERNEL_DS;


    //in the previous kernel space, get the ebp for that kernel space. This represents the ebp frame for the execute that occurred right before the assembly linkage which called execute. idea is we want to go back to assembly linkage
    

    process_list[current_pid] = -1;
    current_pid = parent_pid;
    terminal_info[scheduling_num].Pcount--;
    
    int reternval = newStatus & 0xFF; // 0xFF mask to get the lowest 16 bits

    sti();
    halt_handler(parent_pcb->parent_ebp, parent_pcb->parent_esp, reternval);
    //the old_ebp points to execute frame in old kernel space, want to to pop this frame so we can go into assembly linkage frame and return to parent user process
    //now we will return to parent process's assembly linkage which will return to parent process
    // asm("movl %0, %%ebp" : "=r" (old_ebp));

      
   
    return 0;

}


/* 
 * write
 *   DESCRIPTION: Writes data to a file or device.
 *   INPUTS: fd - file descriptor index
 *           buf - pointer to the buffer containing data to be written
 *           nbytes - number of bytes to write
 *   OUTPUTS: none
 *   RETURN VALUE: Number of bytes written on success, -1 on failure
 *   SIDE EFFECTS: May modify file or device
 */
int32_t write(int32_t fd, const void* buf, int32_t nbytes) {
    cli();
    // Function definition for write
    if(fd < 0 || fd >= 8 || buf == NULL)
    {
        sti();
        return -1;
    }

    pcb_t * temp_pcb = (pcb_t *)(get_pcb_using_pid(current_pid));
    
    int32_t number_of_bytes = -1;
    if(temp_pcb->fda[fd].flags != 0 && temp_pcb->fda[fd].file_jump_table->write_handle != NULL)
    {

        number_of_bytes = temp_pcb->fda[fd].file_jump_table->write_handle(fd, buf, nbytes);
    }
    //writertc(0, buf, 4);
    sti();
    return number_of_bytes;


}


/* 
 * close
 *   DESCRIPTION: Closes the file descriptor.
 *   INPUTS: fd - file descriptor index
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: Closes the file descriptor
 */
int32_t close(int32_t fd) {
     // Function definition for close
    cli();
    if(fd < 2 || fd >= 8){ //trying to close invalid file outside the array range 
        sti();
        return -1; 
    }
    
    pcb_t * temp_pcb = (pcb_t*)(get_pcb_using_pid(current_pid));
    
    if(temp_pcb->fda[fd].flags == 0) //already closed do nothing
    {
        sti();
        return -1;
    }
    temp_pcb->fda[fd].flags = 0;
    temp_pcb->fda[fd].file_pos = 0;

    
    sti();
    return 0;

}

/* 
 * getargs
 *   DESCRIPTION: Retrieves the command line arguments of the current process.
 *   INPUTS: buf - pointer to the buffer where arguments will be stored
 *           nbytes - size of the buffer
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: Modifies buf
 */
int32_t getargs(uint8_t* buf, int32_t nbytes) {
    // Function definition for getargs
   
    if(nbytes<=0)
    {
        return -1;
    }

    pcb_t *pcb = (pcb_t *)(get_pcb_using_pid(current_pid));
    int arg_size = strlen((int8_t * )pcb->argument);

    if(pcb->argument == NULL) //no arguments
    {
        return -1;
    }
    
    if(arg_size > nbytes || arg_size == 0) // buffer too small
    {
        return -1;
    }

    memcpy(buf, pcb->argument, 128); // copy over all 128 bytes of arg

    return 0;
}

/* 
 * execute
 *   DESCRIPTION: Executes the specified program.
 *   INPUTS: command - pointer to the command to execute
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: Starts a new process
 */
int32_t vidmap(uint8_t** screen_start) {
   
cli();


if((int)screen_start==NULL || (int)screen_start == 0x0 || (int)screen_start == MB_4)
    {
        sti();
        return -1;
    }


    int32_t temp = ((int32_t)screen_start);
    if(temp<0x8400000)
    {
        if(temp>=0x8000000)
        {
            //pcb_t *pcb = get_pcb_using_pid(current_pid);
            //set up paging for pcbp
            //set up vidmap paging TODO
             *screen_start = (uint8_t*) 0x8400000;
                set_vidmap_memory(0XB8000);
                flush_tlb();

            sti();
            return 0;
        }
    }
    else if(temp>=0x8000000)
    {
        if(temp<0x8400000)
        {
            //pcb_t *pcb = get_pcb_using_pid(current_pid);
            //set up paging for pcb->pid
            //set up vidmap paging TODO
             *screen_start = (uint8_t*) 0x8400000;
                set_vidmap_memory(0XB8000);
                flush_tlb();

            sti();
            return 0;
        }
    }
    else
    {
        sti();
        return -1;
    }






    //need to do bounds checking

    sti();
    return 0;


}

/* 
 * execute
 *   DESCRIPTION: Executes the specified program.
 *   INPUTS: command - pointer to the command to execute
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: Starts a new process
 */
uint8_t program[64];   // we assume that the command word will never be more than 64 characters
file_ops tempStdin;
file_ops tempStdout;
int flag;
int32_t execute (const uint8_t* command){
    cli();
    int older_pid = current_pid;
    int i;
    int space_count = 0;
    memset(program,0,64);
    uint8_t arguments_temp[128] = {};
    //memset(arguments_temp, '\0', 128);
    uint8_t * arg_temp = arguments_temp;
    //int program_index = 0;
    for(i = 0; i < strlen((int8_t *)command); i++){
        if((int)command[i] == ' '){
            space_count++;

        }

        else if(space_count == 0)
        {
            program[i] = command[i];
        }
        else{
           *arg_temp = command[i];
           arg_temp++;   // need to create new pcb 
            continue;   
        }
    }
    

    if(open_file(program) == -1){ //check if filename doesnt exist
        sti();
        return -1;
    }
    // if(strncmp(program, "fish", 4) == 0){
    //     terminal_info[terminal_num].fish_flag = terminal_num;
    // }
    

    flag = 1;
    for(i = 0; i < MAX_NUM_PROCESSES; i++) 
    {
        if(process_list[i] == -1) //find an unused pid 
        {
            current_pid = i;
            process_list[i] = i;
            flag = 0;
            break;
        }
    }
    if(flag == 1){
        sti();
        return -1;
    }

    
    pcb_t * temp = (pcb_t*)get_pcb_using_pid(current_pid);
    *arg_temp = '\0';    
    memcpy(temp->argument,arguments_temp, 128); //128 is max argument size 

    
    uint8_t temp_buf[4]; //first 4 bytes indicate if executable or not
    read_data(dentry_temp.inode_num, 0, temp_buf, 4);
    if(temp_buf[0] != 0x7f || temp_buf[1] != 0x45 ||  temp_buf[2] != 0x4c || temp_buf[3] != 0x46){ // check for file validity, masks used to check the first 4 bytes of file EOI
        sti();
        return -1;
    }



    set_process_memory(current_pid); //set the virtual page to be the correct  pid
    flush_tlb(); //update tlb

    //user level program loader, want to load executable starting at virtual address 0x08048000
    //so need to get contents of file using buffer
    uint8_t executable_contents[BLOCKS_SIZE];
    open_file((uint8_t*)program); // init/open the file 
    read_data(dentry_temp.inode_num, 0, (uint8_t *)0x08048000, file_size);  //read file contents and write to the address, 0x08048000 is the virtual mem 128mB address 
    read_data(dentry_temp.inode_num, 0, (uint8_t *)executable_contents, BLOCKS_SIZE);
    //EIP where executable starts is in bytes 24-27
    uint8_t * executable_eip = (uint8_t*)((executable_contents[27] << 24) + (executable_contents[26] << 16) + (executable_contents[25] << 8) + executable_contents[24]);
   
    //loader(executable_contents, 0x08048000, BLOCKS_SIZE);


    //process control block
    //this part goes in kernel space (the pcb and kernel stacks, so can use virtual addresses regularly)
    
    //setup_fda(temp->fda);
     //populate stdin
    
    tempStdin.open_handle = NULL;
    tempStdin.close_handle = NULL;
    tempStdin.write_handle = NULL;
    tempStdin.read_handle = readterminal;
   

    
    tempStdout.open_handle = NULL;
    tempStdout.close_handle = NULL;
    tempStdout.write_handle = writeterminal;
    tempStdout.read_handle = NULL;

    temp->fda[0].inode_num = 0;
    temp->fda[0].flags = 1;
    temp->fda[0].file_pos = 0;

    temp->fda[0].file_jump_table = &tempStdin;
    

    temp->fda[1].inode_num = 0;
    temp->fda[1].flags = 1;
    temp->fda[1].file_pos = 0;

    temp->fda[1].file_jump_table = &tempStdout;
    
    
    for(i = 2; i < 8; i++)
    {
        temp->fda[i].flags = 0;
    }

    //memcpy(temp->fda, file_array, 8 * sizeof(fd_t));
    asm("movl %%ebp, %0" : "=r" (temp->parent_ebp));
    asm("movl %%esp, %0" : "=r" (temp->parent_esp));
    temp->eip = (int)executable_eip; //might need for scheduling
    temp->parent_pid = older_pid;
    terminal_info[scheduling_num].pid = current_pid;
    terminal_info[scheduling_num].Pcount++;

    
    //EIP - we have this, it is specified in the executable
    //CS - user mode code, appropriate entry in GDT, in discussion slides it's 0x23
    //SS - stack segment, appropriate entry in GDT, 0x2b
    //DS - need to set this but don't push to stack, also 0x2b
    //ESP - user level stack pointer. Set this to the bottom of the 4 MB page holding the executable image - so 0x083fffc - this is towards bottom of 4 MB page we've reserved for executables

    //also need to do stuff for the PCB and kernel stack

    //also need to enable interrupts

    //skip ahead to context switch, modify the TSS
    tss.esp0 = MB_8 - (current_pid * 0x2000) - 4;     // 0x2000 is for the 4kiB and the 4 is esp offset required
    tss.ss0 = KERNEL_DS;
    sti();
    setUpStack(executable_eip);

    return 0;

    

}

/* 
 * loader
 *   DESCRIPTION: Loads data from source into destination.
 *   INPUTS: source - pointer to the source data
 *           destination - pointer to the destination location
 *           numBytes - number of bytes to load
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: Modifies destination
 */
int32_t loader(uint8_t * source, uint8_t * destination, int numBytes){
    int i;
    for(i = 0; i < numBytes; i++){
        destination[i] = source[i];

    }
    return 0;

}


file_ops tempFilePointers1;
file_ops tempFilePointers2;
file_ops tempFilePointers3;
/* 
 * open
 *   DESCRIPTION: Opens a file for reading, writing, or execution.
 *   INPUTS: filename - pointer to the name of the file to open
 *   OUTPUTS: none
 *   RETURN VALUE: Index of the file descriptor on success, -1 on failure
 *   SIDE EFFECTS: Modifies file descriptors
 */
int32_t open (const uint8_t* filename){
    cli();
    int flag;
    if(-1 == open_file(filename))
    {
        sti();
        return -1; 
    }
   
    pcb_t * pcb_temp = (pcb_t*)(get_pcb_using_pid(current_pid));
    fd_t * file_array = pcb_temp->fda;
    file_array+=2; //skip stdin, stdout
    int i;
    flag = 0;
    for(i = 2; i < 8; i++){ //will run 6 times since first 2 slots taken by stdin, stdout
        if(file_array->flags == 1){
            
            file_array++;
            continue;
        }
        else{
            file_array->file_pos = 0;
            file_array->flags = 1;
            flag = 1;
            break;
        }
    }
    if(flag == 0){
        sti();
        return -1;
    }

    // create file pointers for the structs
    if(dentry_temp.file_type == 2){
        tempFilePointers1.read_handle = read_file;
        tempFilePointers1.open_handle = open_file;
        tempFilePointers1.write_handle = write_file;
        tempFilePointers1.close_handle = close_file;
        file_array->inode_num = dentry_temp.inode_num;
        file_array->file_jump_table = &tempFilePointers1;
    }
    else if(dentry_temp.file_type == 1){
        tempFilePointers2.read_handle = read_directory;
        tempFilePointers2.open_handle = open_directory;
        tempFilePointers2.write_handle = write_directory;
        tempFilePointers2.close_handle = close_directory;
        file_array->inode_num = 0;
        file_array->file_jump_table = &tempFilePointers2;
    }
     else if(dentry_temp.file_type == 0){                   // RTC functions may need to change to fit the jump table arguments

        tempFilePointers3.read_handle = readrtc;
        tempFilePointers3.open_handle = openrtc;
        tempFilePointers3.write_handle = writertc;
        tempFilePointers3.close_handle = closertc;
        file_array->inode_num = 0;
        file_array->file_jump_table = &tempFilePointers3;
    }
   //openrtc("temp");

    sti();
    return i;

}


/* 
 * read
 *   DESCRIPTION: Reads data from a file or device into a buffer.
 *   INPUTS: fd - file descriptor index
 *           buf - pointer to the buffer where data will be stored
 *           nbytes - number of bytes to read
 *   OUTPUTS: none
 *   RETURN VALUE: Number of bytes read on success, -1 on failure
 *   SIDE EFFECTS: Modifies buf
 */
int32_t read (int32_t fd, void* buf, int32_t nbytes){
    cli();
    if(fd < 0 || fd >= 8){
        sti();
        return -1;
    }
    if(nbytes < 0){
        sti();
        return -1;
    }
    if(buf == NULL){
        sti();
        return -1;
    }
    pcb_t * pcb_temp = (pcb_t*)(get_pcb_using_pid(current_pid));
    

    int32_t bytesRead = 0;
    if(pcb_temp->fda[fd].flags != 0 && pcb_temp->fda[fd].file_jump_table->read_handle != NULL )
    {
        bytesRead = pcb_temp->fda[fd].file_jump_table->read_handle(fd, buf, nbytes);
    }
    else
    {
        sti();
        return -1;
    }
    if(fd > 1)
    {
        pcb_temp->fda[fd].file_pos += bytesRead;
    }
    sti();
    return bytesRead;   // n bytes for file, 0 for directory
}

/* 
 * set_handler
 *   DESCRIPTION: Sets a handler for a specific signal.
 *   INPUTS: signum - signal number
 *           handler_address - address of the signal handler function
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: Modifies signal handling behavior
 */
int32_t set_handler(int32_t signum, void* handler_address) {
    // Function definition for set_handler
    return -1;
}

/* 
 * sigreturn
 *   DESCRIPTION: Restores the state of the process before handling the signal.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: Restores process state
 */
int32_t sigreturn(void) {
    // Function definition for sigreturn
    return -1;
}




