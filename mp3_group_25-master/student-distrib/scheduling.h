#ifndef SCHEDULING_H
#define SCHEDULING_H


#include "syscall.h"



volatile uint32_t terminal_num;
int scheduling_num;

typedef struct terminal
{
    uint32_t x_pos;
    uint32_t y_pos;
    char savedbuffer[128];
    char * vid_address;
    uint32_t shown;
    uint32_t opened;
    uint32_t pid;
    uint32_t Pcount;
    int flag;
    int fish_flag;
} terminal_t;

terminal_t terminal_info[3];

void scheduling();
extern void initializepit();
extern void handlepit();
extern void initializeterm();
extern int switchterm(uint32_t num);

#endif 

