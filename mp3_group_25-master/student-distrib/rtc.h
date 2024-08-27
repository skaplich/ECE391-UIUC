#ifndef RTC_H
#define RTC_H

#include "i8259.h"
#include "tests.h"

int RTCFLAG;

// extern int testrtc;
volatile int int_rtc; //initialize RTC
extern void initialize_rtc(); //initializes RTC
extern void handler_rtc(); //handler for RTC

int32_t closertc();//close rtc
int32_t openrtc(const uint8_t* filename); //open rtc
int32_t readrtc(uint32_t fd, uint8_t* buf, uint32_t nbytes); //read rtc
int32_t writertc(uint32_t fd, const void * buf, uint32_t freq); //write rtc
int32_t rtcfreq(int32_t freq); //change the freq of the rtc

#endif
