/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define debug(str, ...) \
	printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)


// declare alkl the functions we will create
int tux_init(struct tty_struct* tty);

int tux_set_led(struct tty_struct* tty, unsigned long arg);

int tux_buttons(struct tty_struct* tty, unsigned long arg);

// init 
//set LED
// buttons

//modify handle packet
//modify ioctl


// 1 for this flag means a command has been completed
// 0 means a command has not been completed
int ack_flag;
unsigned char packets[2];

unsigned char button_save_state[3];   // used to recover button data
unsigned char led_saved_state[6];	  // used to rercover LED state data

//array index corresponds to the value in the 
unsigned char masks[10] = {0xE7, 0x06, 0xCB, 0x8F, 0x2E, 0xAD, 0xED, 0x86, 0xEF, 0xAE};

/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
 * tuxctl-ld.c. It calls this function, so all warnings there apply 
 * here as well.
 */
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    unsigned a, b, c;

    a = packet[0]; 
    b = packet[1];  
    c = packet[2];

	switch(a) {
		case MTCP_RESET:
			
			tux_init(tty);
			// we want to recover the old LED state that was saved in the setLED function which unconditionally backs up the leds
			tuxctl_ldisc_put(tty, led_saved_state, 6);
			tuxctl_ldisc_put(tty, button_save_state, 3);
			break;

		case MTCP_BIOC_EVENT:
				//recevied button press, save the data to packets array in index 0 and 1
				packets[0] = b;
				packets[1] = c;
				break;

		case MTCP_ACK:
				//packet received that states previous command was executed properly
				ack_flag = 1;
				break;
		default:
			break;
	}
    //printk("packet : %x %x %x\n", a, b, c);
}





/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/
int 
tuxctl_ioctl (struct tty_struct* tty, struct file* file, 
	      unsigned cmd, unsigned long arg)
{
    switch (cmd) {
		case TUX_INIT:
			return tux_init(tty);
		case TUX_BUTTONS:
			return tux_buttons(tty, arg);
		case TUX_SET_LED:
			if(ack_flag == 0)
				return 0;
			ack_flag = 0;
			return tux_set_led(tty, arg);
		case TUX_LED_ACK:
			return -EINVAL;
		case TUX_LED_REQUEST:
			return -EINVAL;
		case TUX_READ_LED:
			return -EINVAL;
		default:
			return -EINVAL;
    }
}


/* Initializes any variables associated with the driver and returns 0. Assume that
any user-level code that interacts with your device will call this ioctl before any others */
int tux_init(struct tty_struct* tty)
{
	unsigned char packet_buffer[2];

	ack_flag = 0;

	packet_buffer[0] = MTCP_BIOC_ON;
	packet_buffer[1] = MTCP_LED_USR;

	tuxctl_ldisc_put(tty, packet_buffer, 2); // only two bytes to set for initilization

	return 0;
}


/* tux_set_led()
 * Description: This function sets the LED display on the TUX controller according to the specified
 * arguments. It constructs the LED control packet and sends it to the TUX controller.
 * 
 * Parameters:
 * - tty: Pointer to the tty_struct associated with the TUX controller.
 * - arg: Specifies the LED configuration. It should be a 32-bit value where each nibble represents
 *        one of the four LED digits on the TUX controller.
 * 
 * Returns:
 * - 0 on success.
 */
int tux_set_led(struct tty_struct* tty, unsigned long arg)
{
    int i;

    unsigned char final_return[6];

    unsigned char led_half_byte;
    unsigned char decimal_half_byte;

    // The extra data for segments and opcode
    unsigned char byte0 = (arg >> 0) & 0xFF;  // Lowest byte (bits 0-7)
    unsigned char byte1 = (arg >> 8) & 0xFF;  // Byte 1 (bits 8-15)

	unsigned char byte2 = (arg >> 16) & 0xFF; // Byte 2 (bits 16-23)
    //unsigned char byte3 = (arg >> 24) & 0xFF; // Byte 3 (bits 24-31)

    // The numbers for the time 
    unsigned char number3 = (byte1 >> 4) & 0xF;  // Bits 12-15 of byte1
    unsigned char number2 = byte1 & 0xF;         // Bits 8-11 of byte1

    unsigned char number1 = (byte0 >> 4) & 0xF;  // Bits 4-7 of byte0
    unsigned char number0 = byte0 & 0xF;         // Bits 0-3 of byte0 

    led_half_byte = (arg >> 16) & 0xF;            // Recover bits 16-19 of the arg
    decimal_half_byte = (arg >> 24) & 0xF;        // Recover bits 24-27 of the array

    // Initialize final_return array to all zeros
    memset(final_return, 0, sizeof(final_return));

    // First byte is the op code for set led
    final_return[0] = MTCP_LED_SET;  
    // Second byte is the LED's that we need to turn on 
    final_return[1] = byte2;    // This might not be the original? maybe all LED's

    // Now we need to actually write the masks to our return packet
    for(i = 0; i < 4; i++)  // Maximum 4 LED's we must write to the packet
    {
        if(((led_half_byte >> i) & 0x1) == 1)  // 0x1 is the mask we need to recover a specific bit from the half byte that stores our LED's
        {
            // Writing the LED segment mask to the packet based on whether or not that corresponding LED bit is set to 1 in led_half_byte
            switch(i) {
                case 0:
                    final_return[i+2] = masks[number0];  
                    break;
                case 1:
                    final_return[i+2] = masks[number1];  
                    break;
                case 2:
                    final_return[i+2] = masks[number2];  
                    break;
                case 3:
                    final_return[i+2] = masks[number3];  
                    break;
                default:
                    break;
            }
        }
        if(((decimal_half_byte >> i) & 0x1) == 1)
        {                                       
            final_return[i+2] |= 0x10;  // Mask for selecting a dot bit
        }
    }

    // Save the LED state to our global buffer   
    memcpy(led_saved_state, final_return, sizeof(final_return));

    // Send packet to TUX
    tuxctl_ldisc_put(tty, final_return, 6);
    return 0;
}



/* tux_buttons()
 * Description: This function processes the button press events received from the TUX controller
 * and generates the corresponding output byte indicating the state of the buttons. It constructs
 * the output byte based on the button press status and stores it in the user-provided buffer.
 * 
 * Parameters:
 * - tty: Pointer to the tty_struct associated with the TUX controller.
 * - arg: Pointer to the user-provided buffer where the output byte will be stored.
 * 
 * Returns:
 * - 0 on success.
 * - -EINVAL if there was an error copying data to the user buffer.
 */
int tux_buttons(struct tty_struct* tty, unsigned long arg)
{
	unsigned char byte1 = packets[0];
	unsigned char byte2 = packets[1];

	/* ; 	Response packet:
	;		byte 0 - MTCP_POLL_OK
	;		byte 1  __7_____4___3___2___1_____0____
	;			| 1 X X X | C | B | A | START |
	;			-------------------------------
	;		byte 2  __7_____4_____3______2______1_____0___
	;			| 1 X X X | right | down | left | up |
	;			-------------------------------------- */

	const unsigned char button_masks[4] = {0x8, 0x4, 0x2, 0x1};   // bit 3, 2, 1, 0 masks to select from the byte

	// right left down up c b a start
	unsigned char output = 0xFF;

 	if((byte1 & button_masks[0]) == 0)  //check for C
	{
		output = output & 0xF7;  //set the bit of the button in the output to low (0)
	}
	if((byte1 & button_masks[1]) == 0)  //check for B
	{
		output = output & 0xFB;
	}
	if((byte1 & button_masks[2]) == 0)  //check for A
	{
		output = output & 0xFD;
	}
	if((byte1 & button_masks[3]) == 0)  //check for START
	{
		output = output &  0xFE;
	} 


	if((byte2 & button_masks[0]) == 0)  //check for right
	{
		output = output & 0x7F;
	}
	if((byte2 & button_masks[1]) == 0)  //check for down
	{
		output = output & 0xDF;
	}
	if((byte2 & button_masks[2]) == 0)  //check for left
	{
		output = output & 0xBF;
	}
	if((byte2 & button_masks[3]) == 0)  //check for up
	{
		output = output & 0xEF;
	}


	button_save_state[0] = MTCP_BIOC_EVENT;
	button_save_state[1] = byte1;
	button_save_state[2] = byte2;

	// set lowest byte to the output byte thing that we made up above
	if(copy_to_user((unsigned long *) arg, &output, sizeof(output)) == 0)
	{
		return 0; 
	}
	return -EINVAL;


}
