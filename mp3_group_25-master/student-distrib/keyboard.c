#include "keyboard.h"
#include "i8259.h"
#include "lib.h"
#include "terminaldriver.h"
#include "scheduling.h"

//flags to see if shift, ctrl, caps are pressed
uint8_t lshift = 0;
uint8_t rshift = 0;
uint8_t ctrl = 0;
uint8_t caps = 0;
uint8_t alt = 0;

//flag to see if enter is pressed
volatile static int enter = 0;
static int count = 0; //count of chars pressed
static char buf[128];//buffer to store chars that are outputted

int entered1 = 0;
//num of chars
int num = 0;
int inbound = 0; //checks if char pressed is a letter, needed for caps and shift part



char scannedvals[58][2]= //array of values based on scancodes
{
    {0x0, 0x0}, {0x0, 0x0}, 
    {'1', '!'}, {'2', '@'}, {'3', '#'}, {'4', '$'}, {'5', '%'}, 
    {'6', '^'}, {'7', '&'}, {'8', '*'}, {'9', '('}, {'0', ')'}, 
    {'-', '_'}, {'=', '+'}, 
    {0x08, 0x08}, {' ', ' '}, //backspace
    {'q', 'Q'}, {'w', 'W'}, {'e', 'E'}, {'r', 'R'}, {'t', 'T'}, 
    {'y', 'Y'}, {'u', 'U'}, {'i', 'I'}, {'o', 'O'}, {'p', 'P'}, 
    {'[', '{'}, {']', '}'}, {0x0A, 0x0A}, {0x0, 0x0}, //enter, ctrl
    {'a', 'A'}, {'s', 'S'}, {'d', 'D'}, {'f', 'F'}, {'g', 'G'}, 
    {'h', 'H'}, {'j', 'J'}, {'k', 'K'}, {'l', 'L'}, 
    {';', ':'}, {'\'', '"'}, {'`', '~'}, {0x0, 0x0}, {'\\', '|'}, //shift
    {'z', 'Z'}, {'x', 'X'}, {'c', 'C'}, {'v', 'V'}, {'b', 'B'}, 
    {'n', 'N'}, {'m', 'M'}, {',', '<'}, {'.', '>'}, {'/', '?'}, 
    {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {' ', ' '}//shift 
};



/* initialize_keyboard
 *
 * Inputs: None
 * Outputs: none
 * Side Effects: None
 * Description: initializing the keyboard and allowing interrupts to be send 
 */
void initialize_keyboard(void)//initializing the keybaord
{
    cli();//clearing interrupts
    enable_irq(1);//keyboard irq
}

/* handler_keyboard
 *
 * Inputs: None
 * Outputs: none
 * Side Effects: writing to the screen 
 * Description: taking in scancode and printing out the characters based on the scancodes and 
 * other keys pressed such as shift, ctrl, caps, enter
 */
extern void handler_keyboard(void) //keybard handler
{
    cli();//clearing interrupts
    
    int temp= 0;//checks to see if shift, ctrl, caps is pressed 
    uint8_t scancode = inb(0x60);

    if(scancode==0x2A)//left shift
    {
        lshift=1;//pressed
        temp= 1;
    }
    if(scancode==0xAA)
    {
        lshift=0;//released
        temp=1;
    }
    if(scancode==0x36)//right shift
    {
        rshift=1;//pressed
        temp=1;
    }
    if(scancode==0xB6)
    {
        rshift=0;//released
        temp=1;
    }
    if(scancode==0x1D)//ctrl 
    {
        ctrl=1;//pressed
        temp=1;
    }
    if(scancode==0x9D)
    {
        ctrl=0;//released
        temp=1;
    }
    if(scancode==0x3A)//caps
    {
        caps=!caps;//pressed
        temp=1;
    }

    if(scancode==0x38)
    {
        alt=1;
        temp=1;
       
    }
    if(scancode==0xB8)
    {
        alt=0;
        temp=1;
    }

    if(alt==1)
    {
        if(scancode==0x3B)
        {
            count = 0;
            clearbuf();
            send_eoi(1);
            //setenter1(0);
            sti();
            switchterm(0);
        }
        if(scancode==0x3C)
        {
            count = 0;
            clearbuf();
            send_eoi(1);
            //setenter1(0);
            sti();
            switchterm(1);
        }
        if(scancode==0x3D)
        {
            count = 0;
            clearbuf();
            send_eoi(1);
            //setenter1(0);
            sti();
            switchterm(2);
        }
    }

    if(temp==1)//if one of shift, ctrl, caps pressed send eoi so you can print new char
    {
        send_eoi(1);//keybard irq
        sti();//set interrupts 
        return;
    }

    if(scancode>=58)//invalid scancode
    {
        send_eoi(1);//keyboard irq 
        sti();//set interrupts
        return;
    }
    if(scancode<=1)//invalid scancode
    {
        send_eoi(1);//keyboard irq
        sti();//set interrupts
        return;
    }

    

    //case for when enter is pressed
    if(scannedvals[scancode][0]=='\n')//enter
    {
        num=0;//num of chars in the line goes back to 0
        putc(scannedvals[scancode][0]);//write character to screen
        enter=1;//set enter flag to 1
        if(count>128)//if too many letters go to next line
        {
            buf[127]='\n';//new line since too many letters
            terminal_info[terminal_num].savedbuffer[127]='\n';
        }
        else
        {
            buf[count]='\n';//new line anyway because enter pressed
            terminal_info[terminal_num].savedbuffer[count]='\n';

        }

        if(get_screen_y()>get_num_rows())
        {
            scrolling(); //scrolling function when you go past y limit
        }
    }
    else if(scannedvals[scancode][0]==0x08)//backspace
    {
        if(num>0)//check if there are chars to delete
        {
            // putc('\b \b');//write character to screen
            count=count-1;//dec count and num because of backspace
            num=num-1;
            //char b = (char)('/b');
            buf[num]=NULL;//set to null for backspace
            terminal_info[terminal_num].savedbuffer[num]=NULL;


            cursorback(); //moves cursor back one position 

            putc(' '); ///replace character with space

            cursorback(); //moves cursor back one position

        }
    }
    else if(ctrl>0) //ctrl+L case for clearing the screen
    {
        if(scannedvals[scancode][0]=='l')
        {
            clear_screen(); //clears screen and moves cursor to top left
        }
        if(scannedvals[scancode][0]=='L')
        {
            clear_screen();//clears screen and moves cursor to top left
        }
    }
    else if(lshift==1)//if shift is pressed
    {
        if(num<127)//check if not overflowing over line 
        {
            bounds(scancode);//see if character is within letters range
            if(inbound==1)
            {
                if(caps==1)
                {
                    putc(scannedvals[scancode][0]);//write character to screen
                    if(count<127)//check if not overflowing over buffer 
                    {
                        buf[count]=scannedvals[scancode][0];//update value in buffer based on scancode
                        terminal_info[terminal_num].savedbuffer[count]=buf[count];

                        count=count+1;

                    }
                    else
                    {
                        count=count+1;
                    }
                }
                else
                {
                    putc(scannedvals[scancode][1]);//write character to screen
                    if(count<127)//check if not overflowing over line 
                    {
                        buf[count]=scannedvals[scancode][1];//update value in buffer based on scancode
                        terminal_info[terminal_num].savedbuffer[count]=buf[count];

                    }
                    count=count+1;
                    num=num+1;
                }

            }
            else
            {
                if(caps==1)
                {
                    putc(scannedvals[scancode][1]);//write character to screen
                    if(count<127)//check if not overflowing over buffer 
                    {
                        buf[count]=scannedvals[scancode][1];//update value in buffer based on scancode
                        terminal_info[terminal_num].savedbuffer[count]=buf[count];

                        count=count+1;

                    }
                    else
                    {
                        count=count+1;
                    }
                }
                else
                {
                    putc(scannedvals[scancode][1]);//write character to screen
                    if(count<127)//check if not overflowing over line 
                    {
                        buf[count]=scannedvals[scancode][1];//update value in buffer based on scancode
                        terminal_info[terminal_num].savedbuffer[count]=buf[count];

                    }
                    count=count+1;
                    num=num+1;
                }
            }
        }
    }
    else if(rshift==1)//if shift is pressed
    {
        if(num<127)//check if not overflowing over line 
        {
            bounds(scancode);//see if character is within letters range
            if(inbound==1)
            {
                if(caps==1)
                {
                    putc(scannedvals[scancode][0]);//write character to screen
                    if(count<127)//check if not overflowing over buffer 
                    {
                        buf[count]=scannedvals[scancode][0];//update value in buffer based on scancode
                        terminal_info[terminal_num].savedbuffer[count]=buf[count];

                        count=count+1;

                    }
                    else
                    {
                        count=count+1;
                    }
                }
                else
                {
                    putc(scannedvals[scancode][1]);//write character to screen
                    if(count<127)//check if not overflowing over line 
                    {
                        buf[count]=scannedvals[scancode][1];//update value in buffer based on scancode
                        terminal_info[terminal_num].savedbuffer[count]=buf[count];

                    }
                    count=count+1;
                    num=num+1;
                }

            }
            else
            {
                if(caps==1)
                {
                    putc(scannedvals[scancode][1]);//write character to screen
                    if(count<127)//check if not overflowing over buffer 
                    {
                        buf[count]=scannedvals[scancode][1];//update value in buffer based on scancode
                                             terminal_info[terminal_num].savedbuffer[count]=buf[count];

                        count=count+1;
                    }
                    else
                    {
                        count=count+1;
                    }
                }
                else
                {
                    putc(scannedvals[scancode][1]);//write character to screen
                    if(count<127)//check if not overflowing over line 
                    {
                        buf[count]=scannedvals[scancode][1];//update value in buffer based on scancode
                                             terminal_info[terminal_num].savedbuffer[count]=buf[count];

                        
                    }
                    count=count+1;
                    num=num+1;
                }
            }
        }
    }
    //case if caps is pressed
    else if(caps==1)
    {
        if(num<127)//check if not overflowing over line 
        {
            bounds(scancode);//see if character is within letters range
            if(inbound==1)
            {
                if(lshift==1)
                {
                    putc(scannedvals[scancode][0]);//write character to screen
                    if(count<127)//check if not overflowing over buffer 
                    {
                        buf[count]=scannedvals[scancode][0];//update value in buffer based on scancode
                                             terminal_info[terminal_num].savedbuffer[count]=buf[count];

                        count=count+1;
                    }
                    else
                    {
                        count=count+1;
                    }
                }
                else if(rshift==1)
                {
                    putc(scannedvals[scancode][0]);//write character to screen
                    if(count<127)//check if not overflowing over buffer 
                    {
                        buf[count]=scannedvals[scancode][0];//update value in buffer based on scancode
                                             terminal_info[terminal_num].savedbuffer[count]=buf[count];

                        count=count+1;
                    }
                    else
                    {
                        count=count+1;
                    }
                }
                else
                {
                    putc(scannedvals[scancode][1]);//write character to screen
                    if(count<127)//check if not overflowing over line 
                    {
                        buf[count]=scannedvals[scancode][1];//update value in buffer based on scancode
                                             terminal_info[terminal_num].savedbuffer[count]=buf[count];

                        
                    }
                    count=count+1;
                    num=num+1;
                }

            }
            else
            {
                if(lshift==1)
                {
                    putc(scannedvals[scancode][1]);//write character to screen
                    if(count<127)//check if not overflowing over buffer 
                    {
                        buf[count]=scannedvals[scancode][1];//update value in buffer based on scancode
                                             terminal_info[terminal_num].savedbuffer[count]=buf[count];

                        count=count+1;
                    }
                    else
                    {
                        count=count+1;
                    }
                }
                else if(rshift==1)
                {
                    putc(scannedvals[scancode][1]);//write character to screen
                    if(count<127)//check if not overflowing over buffer 
                    {
                        buf[count]=scannedvals[scancode][1];//update value in buffer based on scancode
                                             terminal_info[terminal_num].savedbuffer[count]=buf[count];

                        count=count+1;
                    }
                    else
                    {
                        count=count+1;
                    }
                }
                else
                {
                    putc(scannedvals[scancode][0]);//write character to screen
                    if(count<127)//check if not overflowing over line 
                    {
                        buf[count]=scannedvals[scancode][0];//update value in buffer based on scancode
                                             terminal_info[terminal_num].savedbuffer[count]=buf[count];

                        
                    }
                    count=count+1;
                    num=num+1;
                }           
            }
        }
    }
    //regular letter with nothing else pressed
    else if(num<127)//see if in same line
    {
        putc(scannedvals[scancode][0]);//write character to screen
        if(count<127)//check if not overflowing over line
        {
            buf[count]=scannedvals[scancode][0];//update value in buffer based on scancode
                                 terminal_info[terminal_num].savedbuffer[count]=buf[count];

        }
        count=count+1;
        num=num+1;
    }
    else if(num>=127)
    {
        num=0;//num of chars in the line goes back to 0
        putc('\n');
        putc(scannedvals[scancode][0]);//write character to screen
        enter=1;//set enter flag to 1
        if(count>=128)//if too many letters go to next line
        {
            buf[127]='\n';//new line since too many letters
                                 terminal_info[terminal_num].savedbuffer[count]=buf[127];

        }
        else
        {
            buf[count]='\n';//new line anyway because enter pressed
                                 terminal_info[terminal_num].savedbuffer[count]=buf[count];

        }
    }


    
    send_eoi(1);//send eoi for irq 1
    sti();//set interrupts

}

/* clearbuf
 *
 * Inputs: none
 * Outputs: none
 * Side Effects: none
 * Description: clears the buf of chars to all 0
 */
void clearbuf()
{
    int i;
    for(i=0; i<=127; i++)
    {
        buf[i]=0;//filling the buf with 0s
        
    }
}
void setbuf(char * newbuf){
    int i;
    for(i = 0; i < 128; i++){
        buf[i] = newbuf[i];
    }
}

/* readbuf
 *
 * Inputs: i, index for value to return from buf
 * Outputs: buf[i], value from buf based on index i
 * Side Effects: none
 * Description: taking in index value and return buf value at that index
 */
char readbuf(int i)
{
    return terminal_info[scheduling_num].savedbuffer[i];//return value of the buf
    //return buf[i];
}

/* beingread
 *
 * Inputs: none
 * Outputs: none
 * Side Effects: calls sti
 * Description: calls sti and cli while waiting for enter to be pressed
 */
void beginread()
{
    sti(); //set interrupts
    while(terminal_info[scheduling_num].flag != 1)//if current terminal has an enter pressed for it
    {
      

    }

   
}

/* checkForEnter
 *
 * Inputs: none
 * Outputs: none
 * Side Effects: checks for enter
 * Description: checks for enter to be pressed, if so sets terminal flag to 1
 */
void checkForEnter(){
    sti(); //enable inteurrupts
    while(enter != 1){ //while enter has not been pressed

    }
    terminal_info[terminal_num].flag = 1; //set flag of terminal to 1
}

/* waitForKeyStroke
 *
 * Inputs: none
 * Outputs: none
 * Side Effects: checks for enter
 * Description: only break out if enter has been pressed and on right terminal
 */
void waitForKeyStroke(){
    sti();
    while(!(enter == 1 && terminal_num == scheduling_num)){ //while enter has not been pressed and not on active terminal

    }
}

/* setEnter
 *
 * Inputs: value
 * Outputs: none
 * Side Effects: sets enter
 * Description: sets enter
 */
void setEnter(int num){
    enter = 0;
}


/* reset
 *
 * Inputs: none
 * Outputs: none
 * Side Effects: calls sti
 * Description: resets count of chars in buf and enter flag and calls sti
 */
void reset()
{
    int i;
    for(i = 0; i < 128; i++){
        buf[i] = '\0';
        terminal_info[terminal_num].savedbuffer[i] = '\0';
    }
    count=0;//set count to 0
    enter=0;//set enter to 0
    sti();//set interrupts
    
}



/* bounds
 *
 * Inputs: scancode
 * Outputs: none
 * Side Effects: none
 * Description: taking in scancode and seeing if it is a regular letter being pressed
 * or if it is a special character, needed for shift, caps cases
 */
void bounds(uint8_t scancode)
{
    //check if in between q and p letters
    if(scancode>=0x10)//q
    {
        if(scancode<=0x19)//p
        {
            inbound = 1;
        }
        
    }
    //check if in between a and l letters
    else if(scancode>=0x1E)//a
    {
        if(scancode<=0x26)//l
        {
            inbound = 1;
        }   
        
    }
    //check if in between z and m letters
    else if(scancode>=0x2C)//z
    {
        if(scancode<=0x32)//m
        {
            inbound = 1;
        }
        

    }
    //if not it is not a regular letter being pressed
    else
    {
        inbound = 0;
    }
    
}


