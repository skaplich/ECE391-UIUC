#include "types.h"  //uint
#include "lib.h"    //memcpy


#define BLOCKS_SIZE 4096

typedef struct file_ops{
   
    int32_t (*open_handle)(const uint8_t* fname);   //init data structure and return 0
    int32_t (*close_handle)(uint32_t fd);
    int32_t (*write_handle)(uint32_t fd, const void* buf, uint32_t length); // should do nothing, returns -1
    int32_t (*read_handle)(uint32_t fd, uint8_t* buf, uint32_t length);  

}file_ops;


// DONT TOUCH THIS PLEASE
typedef struct file_array_entry{

    file_ops * file_jump_table;
    int inode_num;
    int file_pos;
    int flags;
}fd_t;

// pcb struct

typedef struct process_control_block{
  fd_t fda[8]; // only 8 files can be open by a process at a time
  int pid;
  int parent_ebp;
  int parent_esp;
  int scheduler_ebp;
  int scheduler_esp;
  int eip;
  int parent_pid;    
  uint8_t argument[128]; // arbitrary max size for an argument passed into a process execute
}pcb_t;



// dentry is a 64B struct that is used in the boot block
typedef struct dentry_entry {
    uint8_t file_name[32];    //name (up to 32 characters, zero-padded, but not necessarily including a terminal EOS
    uint32_t file_type;      // 0 for  user file, 1 for dir, 2 for regular file
    uint32_t inode_num;      // index for getting the inode of this file
    uint8_t reserved[24];      // 24 bytes of reserved memory
} dentry_entry;


// Boot block is a 4kB block in the file system array at index 0
typedef struct boot_block {
    uint32_t num_dir_entries;      //number of directories/files
    uint32_t inodes;        // N = number of inodes
    uint32_t data_blocks;    // D = number of blocks
    uint8_t reserved[52];      // 1 (8bits) * 52 = 52 reserved bytes for the boot_block
    dentry_entry dir_entries[63]; // 63 dir entries possible each 64b
} boot_block;


// inode is a 4kB block in the file system array at some index specified by a directory entry
typedef struct inode_block {
    uint32_t file_length;      //
    uint32_t data_blocks[1023];      // indicies of the blocks used by this inode file
} inode_block;

dentry_entry  dentry_temp;
boot_block * boot_temp;
int current_pid;
inode_block * inode_temp;
int file_size;
int32_t open_file(const uint8_t* fname);   //init data structure and return 0
int32_t close_file(uint32_t fd);
int32_t write_file(uint32_t fd, const void* buf, uint32_t length); // should do nothing, returns -1
int32_t read_file(uint32_t fd, uint8_t* buf, uint32_t length); // calls read data

int32_t open_directory(const uint8_t* fname);
int32_t close_directory(uint32_t fd);
int32_t write_directory(uint32_t fd, const void* buf, uint32_t length);   //does nothing, returns -1
int32_t read_directory(uint32_t fd, uint8_t* buf, uint32_t length);     //calls read data


int32_t read_dentry_by_name(const uint8_t* fname, dentry_entry* dentry);

int32_t read_dentry_by_index(uint32_t index, dentry_entry* dentry);

int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);


extern void flush_tlb();
extern int32_t get_pcb_using_pid(int32_t pid);
