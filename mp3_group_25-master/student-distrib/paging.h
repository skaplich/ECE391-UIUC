#include "types.h"

#define PAGE_TABLE_MAX 1024 //max number of entries in both tables
#define four_KiB_MAX 4096 // the size of an intel page

#define MB_8 0x800000
#define MB_4 0x400000 

#define KERNEL_START 0x400000 // the start of the virtual memory of the kernel
#define KERNEL_START_TRUNC 0x400000>>12 //only want upper 20 bits
//directory entry for 4 MiB entry (depends on size PS bit)
//I don't think Kernel and Video memory are using this entry
typedef struct page_dir_entry_BIG {
    uint8_t present : 1;
    uint8_t read_write : 1;
    uint8_t user_supervisor : 1;
    uint8_t write_through : 1;   
    uint8_t cache_disable : 1;
    uint8_t accessed : 1;
    uint8_t dirty : 1;
    uint8_t page_size : 1;
    uint8_t global : 1;
    uint8_t available : 3;
    uint8_t page_attribute_table : 1;
    uint8_t offset_20_13 : 8;
    uint8_t rsvd : 1;
    uint32_t offset_31_22 : 10;
} __attribute__((packed)) page_dir_entry_BIG;

//directory entry for 4 KiB entry (depends on size PS bit)
typedef struct page_dir_entry {
    uint8_t present : 1;            // P
    uint8_t read_write : 1;         // R/W
    uint8_t user_supervisor : 1;    // U/S
    uint8_t write_through : 1;      // PWT
    uint8_t cache_disable : 1;      // PCD
    uint8_t accessed : 1;           // A

    uint32_t avl_bit : 1;           // AVL
    uint8_t page_size : 1;          // PS (0)
    uint32_t unused_bit : 1;        // Not used because PS = 0

    uint8_t available : 3;          // AVL   available bits that we can use if we need
    uint32_t offset_31_12 : 20;       // Bits 31-12 of address
} __attribute__((packed)) page_dir_entry;



typedef struct page_table_entry {
    uint8_t present : 1;            // P
    uint8_t read_write : 1;         // R/W
    uint8_t user_supervisor : 1;    // U/S
    uint8_t write_through : 1;      // PWT
    uint8_t cache_disable : 1;      // PCD
    uint8_t accessed : 1;           // A
    uint8_t dirty : 1;              // D     // different from directory
    uint8_t page_attribute_table : 1; // PAT
    uint8_t global : 1;             // G
    uint8_t available : 3;          // AVL
    uint32_t offset_31_12 : 20;     // Bits 31-12 of address
} __attribute__((packed)) page_table_entry;

extern void initialize_paging();
extern void set_process_memory(int process_id);
extern void set_vidmap_memory(int num);
extern void set_background_memory(int terminal_num);

//creating the directory, and the two tables that we need
//with additional alignment
page_dir_entry page_directory[PAGE_TABLE_MAX] __attribute__((aligned (four_KiB_MAX)));
page_table_entry video_memory_page[PAGE_TABLE_MAX] __attribute__((aligned (four_KiB_MAX)));
page_table_entry vidmap_page[PAGE_TABLE_MAX] __attribute__((aligned (four_KiB_MAX)));

