#ifndef KEYBOARD_H
#define KEYBOARD_H

//including types.h
#include "types.h"
#include "lib.h"
#include "i8259.h"

//keyboard functions 
void initialize_keyboard(void);//initializing the keyboard
void bounds(uint8_t scancode);//checking if a letter is pressed or a different character
extern void handler_keyboard(void);//handler for the keyboard

//helper functions for the terminal driver
void clearbuf();//clear buf
char readbuf(int i);//read from buf
void beginread();//wait for enter
void reset();//reset vals
void setbuf(char * newbuf);
void setenter1(int temp);

int getEnteredNum();
void waitForKeyStroke();

void setEnter(int num);

#endif
