
#include "scheduling.h"
#include "syscall_handler.h"

/*
 * initializepit
 *   DESCRIPTION: sets up PIT device
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void initializepit()
{
    cli();
    int freq = 1193182 / 100; //formula for rate generation 
    int lower = freq & 0xFF;
    int upper = freq & 0xFF00;
    upper = upper >>8;
    outb(0x36, 0x43); //channel 0, mode
    outb(lower, 0x40); //data port
    outb(upper, 0x40); //data port
    enable_irq(0); //pit irq number
    sti();
}

/*
 * initializepit
 *   DESCRIPTION: handler when called from IDT
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: sends eoi to let PIT keep sending signals
 */
void handlepit()
{
    
    send_eoi(0); //pit irq number
    scheduling(); //start schedling
    

}
/*
 * scheduling
 *   DESCRIPTION: keeps switching execution of different processes
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: sets up pages, switches terminal numbers, changes global PID
 */
void scheduling(){
    

    if(terminal_info[scheduling_num].opened == 0){ //if terminal has not been opened, execute shell
        terminal_info[scheduling_num].opened = 1;
        execute((uint8_t*)"shell");
    }
    
    
    pcb_t * curr = (pcb_t*)get_pcb_using_pid(terminal_info[scheduling_num].pid); //get the PCB this scheudled process corresponds to
    int temp_esp;
    int temp_ebp;
    asm("movl %%ebp, %0" : "=r" (temp_ebp)); //save current ebp and esp in temp variables
    asm("movl %%esp, %0" : "=r" (temp_esp));
    curr->scheduler_ebp = temp_ebp; //add these values into the PCB so other processes can get back to this context
    curr->scheduler_esp = temp_esp;
    int next_index = (scheduling_num + 1)%3; //update scheduling number
   
    scheduling_num = next_index;

    if(terminal_info[next_index].opened == 0){ //if next process has not been opened execute shell
        terminal_info[next_index].opened = 1;
        execute((uint8_t*)"shell");

    }

    

     current_pid = terminal_info[next_index].pid;

    pcb_t * next = (pcb_t*)get_pcb_using_pid(terminal_info[scheduling_num].pid); //get information about process we want to switch to
    int next_esp,next_ebp;
    next_esp = next->scheduler_esp;  //get esp's and ebp's of context we want to switch to
    next_ebp = next->scheduler_ebp;
    set_process_memory(terminal_info[scheduling_num].pid); //set user paging
    flush_tlb();
    tss.esp0 = MB_8 - (terminal_info[scheduling_num].pid * 0x2000) - 4;     // 0x2000 is for the 4kiB and the 4 is esp offset required
    tss.ss0 = KERNEL_DS; //set up kernel stack when next user process is interrupted again
    halt_handler(next_ebp, next_esp,0); //go to the halt handler so we can switch to appropriate context
    
    


}
/*
 * initializeterm
 *   DESCRIPTION: initialize terminal info
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void initializeterm()
{
    cli();
    terminal_num = 0;
    scheduling_num = terminal_num;
    terminal_info[0].x_pos = 0;
    terminal_info[0].y_pos = 0;
    terminal_info[0].vid_address = (char*)0xB8000+2*4096;// background page 

    terminal_info[1].x_pos = 0;
    terminal_info[1].y_pos = 0;
    terminal_info[1].vid_address = (char*)0xB8000+3*4096;// background page

    terminal_info[2].x_pos = 0;
    terminal_info[2].y_pos = 0;
    terminal_info[2].vid_address = (char*)0xB8000+4*4096;// background page
    terminal_info[0].shown = 1; //start on term 0
    terminal_info[1].shown = 0;//keeps track of what terminal is shown
    terminal_info[2].shown = 0;

    terminal_info[0].opened = 0; //start on term 0
    terminal_info[1].opened = 0;
    terminal_info[2].opened = 0;

    terminal_info[0].Pcount = 0; //start on term 0
    terminal_info[1].Pcount = 0;//gets the number of active processes for each terminal, useful for halt
    terminal_info[2].Pcount = 0;
    
    terminal_info[0].flag = 0; //start on term 0
    terminal_info[1].flag = 0;//flags to tell if enter has been pressed
    terminal_info[2].flag = 0;

    terminal_info[0].fish_flag = -1; //start on term 0
    terminal_info[1].fish_flag = -1;
    terminal_info[2].fish_flag = -1;
    sti();
}
/*
 * switchterm
 *   DESCRIPTION: switches terminal
 INPUT: terminal num we want to switch to
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: does 2 video mem to background switches
 */
int switchterm(uint32_t num)
{
    //int i;
        //check if terminal num is valid

    if(num<0 || num>=3)
    {
        return -1;
    }
    if(num==terminal_num)
    {
        return 0;
    }
    
    
    memcpy((uint32_t*)terminal_info[terminal_num].vid_address, (uint32_t*)0xB8000, 4096); //save current screen to corresponding video page
    memcpy((uint32_t*)0xB8000, (uint32_t*)terminal_info[num].vid_address, 4096); //save current screen to corresponding video page
    //terminal_info[num].shown = 1;
    terminal_num = num;
     

   
    
    return 0;
}
