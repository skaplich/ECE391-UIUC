#ifndef SYSCALL_H
#define SYSCALL_H


//delete the includes that we don't need after writing the functions
#include "types.h"
#include "rtc.h"
#include "keyboard.h"
#include "lib.h"
#include "terminaldriver.h"
#include "paging.h"
#include "fileSystem.h"
#include "x86_desc.h"
#define MAX_NUM_PROCESSES 6


int32_t halt(uint8_t status);
int32_t execute (const uint8_t* command);
int32_t read (int32_t fd, void* buf, int32_t nbytes);
int32_t write (int32_t fd, const void* buf, int32_t nbytes);
int32_t open (const uint8_t* filename);
int32_t close (int32_t fd);
int32_t getargs (uint8_t* buf, int32_t nbytes);
int32_t vidmap (uint8_t** screen_start);
int32_t set_handler(int32_t signum, void* handler_address);
int32_t sigreturn(void);




#endif
