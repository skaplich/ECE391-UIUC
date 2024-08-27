#include "terminaldriver.h"
#include "keyboard.h"
#include "lib.h"
#include "scheduling.h"

/* closeterminal
 *
 * Inputs: none
 * Outputs: none
 * Side Effects: none
 * Description: returns 0
 */

int32_t closeterminal()
{
    return 0; //return 0 to clear the terminal
}

/* openterminal
 *
 * Inputs: none
 * Outputs: none
 * Side Effects: clears screen
 * Description: clears screen, clears buf and returns 0
 */
int32_t openterminal()
{
    //clearbuf();//clear the buffer holding text
    //clear_screen();//clear screen and move cursor to top left
    return 0;

}

/* readterminal
 *
 * Inputs:  void* buffer, int32_t n
 * Outputs: reads input to the terminal screen
 * Side Effects: none
 * Description: reads in input from the terminal screen to 
 * print back to terminal screen through write
 */
int32_t readterminal(uint32_t fd, uint8_t* buffer, uint32_t n)
{
    int j;
    for(j =0;j < n; j++)
    {
        buffer[j] = '\0';
    }
   
    waitForKeyStroke(); //wait for enter to be pressed
    //while(1);
    setEnter(0); //reset enter flag back to 0
    cli();
    int i;
    int numread; //holds the number of chars read
   
    terminal_info[terminal_num].flag = 0; //set terminal num flag back to 0
    
    for(i=0; i<=127; i++) //max number of chars in buffer
    {

       
        //storing value from keyboard buffer to terminal buffer
        
        if(readbuf(i)=='\n') //check if enter is pressed
            {
            break; //if pressed leave read 
            }
        
        if(readbuf(i)!='\n') //if enter isnt pressed
        {
            ((char*)buffer)[i]=readbuf(i);//readbuf reads the value from the buffer
            //terminal_info[terminal_num].savedbuffer[i] = ((char*)buffer)[i];
            numread=i+1;//increment num of chars read

            int temp=n-1;
            if(temp==i)//check if buffer is full
            {
                ((char*)buffer)[i]='\n';//put an enter into the buffer anyway
                break;
            }
        } 


    }
    reset();//reset values after reading 
    //terminal_info[scheduling_num].y_pos=get_screen_y();
    //terminal_info[scheduling_num].x_pos=get_screen_x();
    sti();
    return numread;//return number of chars read
}

/* writeterminal
 *
 * Inputs: const void* buffer, int32_t n
 * Outputs: writes output to the terminal screen
 * Side Effects: writes to screen
 * Description: takes in buffer and writes back to the terminal screen
 */
int32_t writeterminal(uint32_t fd, const void* buffer, uint32_t n)
{
    cli();

    uint32_t xval =     terminal_info[scheduling_num].x_pos;

     uint32_t yval =   terminal_info[scheduling_num].y_pos;
    set_screen_x(xval);
    set_screen_y(yval);
    if(buffer==NULL)//check the buffer for bad input 
    {   
        return -1;//return -1 if bad input
    }
    int i;
    int numwrite=0;//number of chars that are being written
    //int temp = n-1;
    //putc(' ');
    for(i=0; i<n; i++)
    {
        if(terminal_info[scheduling_num].opened==1)
        {
            if(terminal_num==scheduling_num)
            {            putc(((char*)buffer)[i]);//print out the character from the buffer
            numwrite=numwrite+1;//increment num of chars written

            }
            else
            {
                putct(((char*)buffer)[i]);//print out the character from the buffer
            numwrite=numwrite+1;//increment num of chars written
            }

        }
    }
//update screen_x and screen_y for the process

    terminal_info[scheduling_num].y_pos=get_screen_y();
    terminal_info[scheduling_num].x_pos=get_screen_x();
        position_cursor(terminal_info[scheduling_num].x_pos, terminal_info[scheduling_num].y_pos); //update cursor

    
    int j;
    for(j = 0; j < 128; j++){
       terminal_info[scheduling_num].savedbuffer[j] = '\0';
    }

    sti();
    return numwrite;//return number of chars written
}

