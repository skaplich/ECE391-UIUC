#include "idt.h"
#include "lib.h"
#include "x86_desc.h"
#include "keyboard.h"
#include "rtc.h"
#include "IntHandler.h"
#include "syscall_handler.h"
#include "syscall.h"
#include "scheduling.h"


/* Our exception handlers with printing support: */

char exceptionArr [20][50] = {"Divided by 0 error\n","debug error\n","NMI error\n","Breakpoint error\n","Overflow error\n","Out of bounds error\n"
,"Opcode error\n", "Device unavailable error\n","Double fault error\n","Coprocessor segment overrun error\n", "Invalid tss error\n", "Segment not there error\n",
  "Stack segfault err\n", "General protection error\n", "Page fault error\n","", "Floating Point error\n", "Alignment error\n", "Machine check error\n",
  "SIMD floating point error\n"};

/* 
 * exception_handler
 *   DESCRIPTION: Handles exceptions by printing an error message corresponding to the given offset,
 *                then halts the system.
 *   INPUTS: offset - the offset of the exception being handled
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: Prints error message and halts the system
 */

void exception_handler (uint32_t offset){
	cli();
	clear();
	printf(exceptionArr[offset]);
	halt(255);
	// while(1){

	// }
	
}

void division_zero()
{
	cli();
	clear();
	printf(" Divided by 0 error\n");
	halt(255);
	// while(1){

	// }

}

void debug_err()
{
	cli();
	clear();
	printf(" debug error\n");
	halt(255);
	// while(1){

	// }
}

void NMI_err()
{
	cli();
	clear();
	printf(" NMI error\n");
	halt(255);
	// while(1){

	// }
}

void breakpoint_err()
{
	cli();
	clear();
	printf(" Breakpoint error\n");
	halt(255);
	// while(1){

	// }
}

void overflow_err()
{
	cli();
	clear();
	printf(" Overflow error\n");
	halt(255);
	// while(1){

	// }
}

void bounds_err()
{
	cli();
	clear();
	printf(" Out of bounds error\n");
	halt(255);
	// while(1){

	// }
}

void opcode_err()
{
	cli();
	clear();
	printf(" Opcode error\n");
	halt(255);
	// while(1){

	// }
}

void device_err()
{
	cli();
	clear();
	printf(" Device unavailable error\n");
	halt(255);
	// while(1){

	// }
}

void doublefault_err()
{
	cli();
	clear();
	printf(" Double fault error\n");
	halt(255);
	// while(1){

	// }
}

void coproc_err()
{
	cli();
	clear();
	printf(" Coprocessor segment overrun error\n");
	halt(255);
	// while(1){

	// }
}

void tss_err()
{
	cli();
	clear();
	printf(" Invalid tss error\n");
	halt(255);
	// while(1){

	// }
}

void segment_err()
{
	cli();
	clear();
	printf(" Segment not there error\n");
	halt(255);
	// while(1){

	// }
	
}

void stack_err()
{
	cli();
	clear();
	printf(" Stack segfault err\n");
	halt(255);
	// while(1){

	// }
}

void general_err()
{
	cli();
	clear();
	printf(" General protection error\n");
	halt(255);
	// while(1){

	// }
}

void page_err()
{
	cli();
	//clear();
	printf(" Page fault error\n");
	halt(255);
	// while(1){

	// }
}

void fp_err()
{
	cli();
	clear();
	printf(" Floating Point error\n");
	halt(255);
	// while(1){

	// }
}


void align_err()
{
	cli();
	clear();
	printf(" Alignment error\n");
	halt(255);
	// while(1){

	// }
}


void machine_err()
{
	cli();
	clear();
	printf(" Machine check error\n");
	halt(255);
	// while(1){

	// }
}


void simd_fp_err()
{
	cli();
	clear();
	printf(" SIMD floating point error\n");
	halt(255);
	// while(1){

	// }
}


void system_call()
{
	cli();
	clear();
	printf(" temporary system call\n");
	halt(255);
	// while(1){

	// }
}
/* 
 * initialize_idt
 *   DESCRIPTION: Initializes the Interrupt Descriptor Table (IDT) with the 
 *                appropriate interrupt handlers and settings for system exceptions,
 *                hardware interrupts, and system calls.
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: Initializes the IDT for interrupt handling
 */

extern void initialize_idt()
{
	int i;
	// 20 is max index of the intel standard interrupts that we will add 
	for(i = 0; i < 20; i++)
	{
		idt[i].present=1;   //every standard Intel interrupt is present
		idt[i].dpl=0;		//should be 3 for system calls
		idt[i].reserved0=0;	//always just 0
		idt[i].reserved1=1;	//usually 1, 0 for task gates
		idt[i].reserved2=1;	//always just 1
		idt[i].reserved3=0;	//set to 1 if interrupt, 0 for exceptions
		idt[i].reserved4=0; //always 0 or ignored - part of [4:0] reserved
		idt[i].size=1;		//always the same 32bit length  (D bit)   (except for task gates)
		idt[i].seg_selector=KERNEL_CS;	//always permission level set by kernel
	}
	
	//all other entries are set, but not present
	for(i = 20; i < NUM_VEC; i++)
	{
		idt[i].present=0;   //outside the defined interrupts - must not be present
		idt[i].dpl=0;		
		idt[i].reserved0=0;	
		idt[i].reserved1=1;	
		idt[i].reserved2=1;	
		idt[i].reserved3=0;	
		idt[i].reserved4=0; 
		idt[i].size=1;		
		idt[i].seg_selector=KERNEL_CS;	
	}

	//intel defined, we will not permit access to this entry
	idt[15].present = 0;



	idt[0x20].present=1;
	idt[0x20].dpl=0;
	idt[0x20].reserved1=1;
	idt[0x20].reserved2=1;
	idt[0x20].reserved3=0;
	idt[0x20].reserved4=0; 
	idt[0x20].size=1;
	idt[0x20].seg_selector=KERNEL_CS;
	SET_IDT_ENTRY(idt[0x20], pitWrap);     //what is this? where is this?

	idt[0x21].present=1;
	idt[0x21].dpl=0;
	idt[0x21].reserved1=1;
	idt[0x21].reserved2=1;
	idt[0x21].reserved3=1;
	idt[0x21].reserved4=0;
	idt[0x21].size=1;
	idt[0x21].seg_selector=KERNEL_CS;
	SET_IDT_ENTRY(idt[0x21], keyboardWrap);            //wrong funtion name? didn't lucky write this in keyboard.c

	idt[0x28].present=1;
	idt[0x28].dpl=0;
	idt[0x28].reserved1=1;
	idt[0x28].reserved2=1;
	idt[0x28].reserved3=1;
	idt[0x28].reserved4=0;
	idt[0x28].size=1;
	idt[0x28].seg_selector=KERNEL_CS;
	SET_IDT_ENTRY(idt[0x28], rtcWrap);				  //wrong funtion name again? check rtc.c file 

	idt[0x80].present=1;
	idt[0x80].dpl=3;				//this is for a sys call only
	idt[0x80].reserved1=1;
	idt[0x80].reserved2=1;
	idt[0x80].reserved3=0;          //this was originally a 1 ?? I change this to 0 because why would it be 1
	idt[0x80].reserved4=0;
	idt[0x80].size=1;
	idt[0x80].seg_selector=KERNEL_CS;
	SET_IDT_ENTRY(idt[0x80], syscall_handler);          //this system call should be a pointer to an assembly label that saves registers and does all that stuff


	//referenced in page 586 of intel manual
	SET_IDT_ENTRY(idt[0x00], division_zero);	//division error
	SET_IDT_ENTRY(idt[0x01], debug_err);  		//debug exception         //this might be reserved
	SET_IDT_ENTRY(idt[0x02], NMI_err);			//NMI interrupt
	SET_IDT_ENTRY(idt[0x03], breakpoint_err);	//breakpoint
	SET_IDT_ENTRY(idt[0x04], overflow_err);		//overflow
	SET_IDT_ENTRY(idt[0x05], bounds_err);		//bound range exceeded
	SET_IDT_ENTRY(idt[0x06], opcode_err);		//invalid opcode
	SET_IDT_ENTRY(idt[0x07], device_err);		//device not available
	SET_IDT_ENTRY(idt[0x08], doublefault_err);	//double fault
	SET_IDT_ENTRY(idt[0x09], coproc_err);		//coprocessor overrun        //might be reserved
	SET_IDT_ENTRY(idt[0x0A], tss_err);			//invalid TSS
	SET_IDT_ENTRY(idt[0x0B], segment_err);		//segment not present
	SET_IDT_ENTRY(idt[0x0C], stack_err);		//stack segment fault
	SET_IDT_ENTRY(idt[0x0D], general_err);		//general protection
	SET_IDT_ENTRY(idt[0x0E], page_err);			//page fault
	//SKIP Entry 15 because it's intel reserved 
	SET_IDT_ENTRY(idt[0x10], fp_err);			//math error
	SET_IDT_ENTRY(idt[0x11], align_err);		//alignment check
	SET_IDT_ENTRY(idt[0x12], machine_err);		//machine check
	SET_IDT_ENTRY(idt[0x13], simd_fp_err);		//SIMD floating point exception //might be reserved 



	/* TODO : */
	//set values and SET IDT ENTRY in the corresponding entries : 
	// SYS CALL
	// KEYBOARD HANDLER
	// PIC LINKAGE
	// HANDLER

	lidt(idt_desc_ptr);
}


//We might still need to do this? :

// Load the interrupt descriptor table (IDT).

// Load the local descriptor table (LDT) register.
