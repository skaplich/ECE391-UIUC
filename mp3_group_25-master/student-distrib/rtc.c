#include "rtc.h"
#include "lib.h"
#include "tests.h"
#include "scheduling.h"

// extern int testrtc;
//initialize_rtc
//DESCRIPTION - initializes RTC
//INPUTS - None
//OUTPUTS - None
//Return val - none
//side effects - send control words to PIC
int rtc_flags[3] = {0,0,0};
void initialize_rtc()
{
	RTCFLAG = 1;
	//rtcfreq(2);

	char first;
	cli();
	outb(0x8B, 0x70);//reg b and rtc select
	first =  inb(0x71);//cmos
	outb(0x8B, 0x70);
	first = first | 0x40; //mask
	outb(first, 0x71);

	outb(0x8A, 0x70);//reg a and rtc select
	first = inb(0x71);
	outb(0x8A, 0x70); //mask
	first = first & 0xF0;
	first = first | 6; //masking
	outb(first, 0x71);

	enable_irq(8);//irq line one
	
}

//handler_rtc
//DESCRIPTION - handler for RTC
//INPUTS - None
//OUTPUTS - None
//Return val - none
//side effects - send control words to PIC
void handler_rtc()
{
	cli();
	//rtcfreq(2);
	//test_freq();
	// if(testrtc==1)
	// {
		//printf("all rtc functions work yay!!!-");
	//}
    //printf("Testing: RTC||");
	
	RTCFLAG = 0;
	outb(0x8C, 0x70); //write to PIC
	inb(0x71);
	rtc_flags[terminal_num]=1;
	int_rtc=1;
	send_eoi(8);
	sti();
}

//openrtc
//DESCRIPTION - calls the function to set freq value for rtc
//INPUTS - None
//OUTPUTS - None
//Return val - none
//side effects - changes frequency of the rtc
int32_t openrtc(const uint8_t* filename)
{
	rtcfreq(2); //set freq to basic 2 
	return 0;
}


//closertc
//DESCRIPTION - returns 0
//INPUTS - None
//OUTPUTS - None
//Return val - 0
//side effects - none
int32_t closertc()
{
	return 0; //return 0
}

//readrtc
//DESCRIPTION - returns 0
//INPUTS - None
//OUTPUTS - None
//Return val - 0
//side effects - none
int32_t readrtc(uint32_t fd, uint8_t* buf, uint32_t nbytes)
{
	// if(fd > 0)
	// {
	// 	return 0;
	// }
	//int_rtc=0;
	sti();
	// while(int_rtc==0) //while rtc interrupt 0
	// {
		
	// }
	// int_rtc=0;
	while(rtc_flags[scheduling_num]==0)
	{

	}
	rtc_flags[scheduling_num]=0;
	return 0;
}

//writertc
//DESCRIPTION - returns -1 if frequency able to change, -1 if not
//INPUTS - freq
//OUTPUTS - new freq or -1 if did not work
//Return val - new freq or -1 if did not work
//side effects - none

int32_t writertc(uint32_t fd, const void * buf, uint32_t freq)
{

	if(rtcfreq(*((int32_t*)buf))==1)
	{
		return *((int32_t*)buf);
	}
	return -1;
}

//rtcfreq
//DESCRIPTION - changes the frequency of the rtc
//INPUTS - freq
//OUTPUTS - None
//Return val - 1 or -1 based on worked or did not work
//side effects - changes frequency of the rtc
int32_t rtcfreq(int32_t freq)
{
    char newfreq;
	char temp;
	int32_t setfreq;
	int mask = 0xF0;
	if(freq > 1024)
	{
		freq = 1024;
	}

	if(freq==2)
	{
		newfreq=0x0F;//rtc 2 value
	}
	else if(freq==4)
	{
		newfreq=0x0E;//rtc 4 value
	}
	else if(freq==8)
	{
		newfreq=0x0D;//rtc 8 value
	}
	else if(freq==16)
	{
		newfreq=0x0C;//rtc 16 value
	}
	else if(freq==32)
	{
		newfreq=0x0B;//rtc 32 value
	}	
	else if(freq==64)
	{
		newfreq=0x0A;//rtc 64 value
	}
	else if(freq==128)
	{
		newfreq=0x09;//rtc 128 value
	}
	else if(freq==256)
	{
		newfreq=0x08;//rtc 256 value
	}
	else if(freq==512)
	{
		newfreq=0x07;//rtc 512 value
	}
	else if(freq==1024)
	{
		newfreq=0x06;//rtc 1024 value
	}	
	else
	{
		return -1;
	}

	//newfreq=0x0D;
    outb(0x8A, 0x70);//set rtc select - reg a
    temp = inb(0x71);//current value of rtc regA
    outb(0x8A, 0x70);//set rtc select - reg a

	temp = temp & mask;
    setfreq = temp | newfreq;
    outb(setfreq, 0x71);//rtc cmos

    return 1;
}
