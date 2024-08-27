/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = 0xFF; /* IRQs 0-7  */ //want to disable all interrupts at beginning
uint8_t slave_mask = 0xFF;  /* IRQs 8-15 */

//i8259_init
//DESCRIPTION - initialize PIC by sending control words
//INPUTS - None
//OUTPUTS - None
//Return val - none
//side effects - allows for interrupts, sets up master and slave PIC
/* Initialize the 8259 PIC */
void i8259_init(void) {


    //mask all irqs at first
    
	outb(0xFF, MASTER_8259_PORT+1);
     outb(0xFF, SLAVE_8259_PORT+1); //masking before init


    outb(ICW1, MASTER_8259_PORT); //push all icw codes
    outb(ICW2_MASTER, MASTER_8259_PORT + 1);
    outb(ICW3_MASTER,MASTER_8259_PORT + 1 ); // master
	outb(ICW4, MASTER_8259_PORT+1);

    outb(ICW1, SLAVE_8259_PORT);
    outb(ICW2_SLAVE, SLAVE_8259_PORT + 1);
    outb(ICW3_SLAVE,SLAVE_8259_PORT + 1 ); // slave
	outb(ICW4, SLAVE_8259_PORT+1);

 

	 enable_irq(2); //want to enable the interrupt 2 corresponding to the slave PIC

}


//enable_irq
//DESCRIPTION - allow the interrupt to be created from a specific device
//INPUTS - None
//OUTPUTS - None
//Return val - none
//side effects - allows for interrupts
/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {

	uint8_t temp;
   if(irq_num<0) //invalid interrupt
   {
	   return;
   }
   if(irq_num>15) //invalid interrupt
   {
	return ;
   }
   if(irq_num<8) //if on master PIC
   {
	temp = 1<<irq_num; //create the mask so that only appropriate interrupt can occur
	master_mask = master_mask & ~temp;
	outb(master_mask, MASTER_8259_PORT+1); //write to master
	return;
   }
   else{
	irq_num -= 8; //create the mask so that only appropriate interrupt can occur
	temp = 1<<irq_num;
	master_mask = master_mask & ~temp;
	outb(master_mask, SLAVE_8259_PORT+1); //write to slave
	return;
   }

}


//disable_irq
//DESCRIPTION - disable PIC interrupt
//INPUTS - None
//OUTPUTS - None
//Return val - none
//side effects - disable interrupt
/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
    if(irq_num >= 0 && irq_num <= 7){
        outb(master_mask, MASTER_8259_PORT+1); //write to master
    }
    else{
        outb(slave_mask, SLAVE_8259_PORT+1); //write to slave
    } 

}


//send_eoi
//DESCRIPTION - tell PIC interrupt is done being serviced
//INPUTS - None
//OUTPUTS - None
//Return val - none
//side effects 
/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
	uint8_t temp;
	if(irq_num<0) //invalid IRQ
	{
		return;
	}
	if(irq_num>15)
	{
		return;
	}
	if(irq_num<8) 
	{
		temp = EOI|irq_num; //to send the EOI
		outb(temp, MASTER_8259_PORT); //write to master
		return;
	}
	temp = EOI|2;
	outb(temp, MASTER_8259_PORT); //to send the EOI
	temp = irq_num-8;
	temp = temp|EOI;
	outb(temp, SLAVE_8259_PORT); //write to master
}
