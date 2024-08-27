
#ifndef INTHANDLER_H
#define INTHANDLER_H

#ifndef ASM

extern void keyboardWrap(); //keyboard linnkage
extern void rtcWrap(); //rtc linkage
extern void pitWrap(); //pit linkage
extern void pagingInit(uint32_t * input); //initialize paging


#endif

#endif
