#include "paging.h"
#include "IntHandler.h"


/*
 * initialize_paging
 *   DESCRIPTION: Initializes the paging mechanism by setting up page directories and tables
 *                to map virtual addresses to physical addresses. Also, sets up control registers
 *                for paging to work.
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: Initializes page directories and tables, sets up control registers
 */
void initialize_paging()
{
    //page_dir_entry dir_page;


    // load one page directory into physical memory, this will be the video mem page
    page_directory[0].present = 1;      
    page_directory[0].read_write = 1;  
    page_directory[0].user_supervisor = 0; // only supervisor can access
    page_directory[0].write_through = 0;   // write-back is enabled, higher preformance
    page_directory[0].cache_disable = 0;    // this enables caching for the page, I'm assuming this is for the TLB
   
    page_directory[0].avl_bit = 0;       // needed if PS = 0
    page_directory[0].page_size = 0;     // page entries should be 4kB each
    page_directory[0].unused_bit = 0;    // needed is PS = 0
   
    page_directory[0].available = 0;     // don't need any temp bits right now
    page_directory[0].offset_31_12 = ((uint32_t)video_memory_page) >> 12;     //video memory page address, must be on 4096 boundaries of physical memory




    // load second page directory into physical memory, this will be the kernel page
    page_directory[1].present = 1;
    page_directory[1].read_write = 1;
    page_directory[1].user_supervisor = 1;    // checkpoint 3 change ?
    page_directory[1].write_through = 0;
    page_directory[1].cache_disable = 0;   // I guess we can cache this page as well


    page_directory[1].avl_bit = 0;
    page_directory[1].page_size = 1;
    page_directory[1].unused_bit = 0;


    page_directory[1].available = 0;      
    page_directory[1].offset_31_12 = KERNEL_START_TRUNC;      // address of the kernel page, needs the highest 20 bits/needs to be on boundary




    // The rest of the directory should be filled but not presentvxvd
    int j;
    for(j = 2; j < PAGE_TABLE_MAX; j++)
    {


        page_directory[j].present = 0;    // most important for the random directory entries
        page_directory[j].read_write = 1;
        page_directory[j].user_supervisor = 0;
        page_directory[j].write_through = 0;
        page_directory[j].cache_disable = 0;   // should we be catching this?


        page_directory[j].avl_bit = 0;
        page_directory[j].page_size = 1;
        page_directory[j].unused_bit = 0;


        page_directory[j].available = 0;      
        page_directory[j].offset_31_12 = 0;  // we don't need this page so I guess address is just zero    
    }


    // fill in the entries of the video memory page so that each 4KiB entry links to physical memory
    // in total the 1024 entries should amount to 4MB of total video memory
    int i;
    for(i = 0; i < PAGE_TABLE_MAX; i++)
    {
        if(i == 184 || i == 186 || i == 187 || i == 188){ // this is 0xB8000 which is the vga physical address divided by 4096 which is the 4k alignment value
        video_memory_page[i].present = 1;
        video_memory_page[i].read_write = 1;
        video_memory_page[i].user_supervisor = 1;
        video_memory_page[i].write_through = 0;
        video_memory_page[i].cache_disable = 0;
        video_memory_page[i].accessed = 0;
        video_memory_page[i].dirty = 0;
        video_memory_page[i].page_attribute_table = 0;
        video_memory_page[i].global = 0;
        video_memory_page[i].available = 0;
       


       }
        else{


        video_memory_page[i].present = 0;
        video_memory_page[i].read_write = 1;
        video_memory_page[i].user_supervisor = 0;
        video_memory_page[i].write_through = 0;
        video_memory_page[i].cache_disable = 0;
        video_memory_page[i].accessed = 0;
        video_memory_page[i].dirty = 0;
        video_memory_page[i].page_attribute_table = 0;
        video_memory_page[i].global = 0;
        video_memory_page[i].available = 0;
        }
       
        video_memory_page[i].offset_31_12 = i;   //start at 0 -> 4 kB ... -> 0x40000     // 4096 = 4 * number of bytes in 1 KiB  
    }
   
    set_process_memory(0);




    /* TODO HERE : */
    //setup control registers given the address
    // need to set specific control register in order for paging to work
    // this should probably be inline assembly!
    pagingInit((uint32_t *)page_directory);


}






/*
 * set_process_memory
 *   DESCRIPTION: Sets up the memory mapping for a given process in the page directory.
 *   INPUTS: process_id -- ID of the process
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Modifies the page directory entries for the process
 */
void set_process_memory(int process_id)
{
    //0x08000000 corresponds to 128MB which is 32 * 4 = 128, thus 31 is the index into our  
    page_directory[32].present = 1;    
    page_directory[32].read_write = 1;
    page_directory[32].offset_31_12 = (MB_8 + MB_4 * process_id)>>12;
    page_directory[32].user_supervisor = 1;
        //flush_tlb();


}



/*
 * set_vidmap_memory
 *   DESCRIPTION: Sets up the memory mapping for a given video memory in the page directory.
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Modifies the page directory entries for the process
 */
void set_vidmap_memory(int num)//    set_vidmap_memory(screen_start);
{
    page_directory[33].offset_31_12 = ((uint32_t)vidmap_page) >> 12;
    page_directory[33].present = 1;    
    page_directory[33].read_write = 1;  
    page_directory[33].user_supervisor = 1;
    page_directory[33].write_through = 0;
    page_directory[33].cache_disable = 0;   // I guess we can cache this page as well


    page_directory[33].avl_bit = 0;
    page_directory[33].page_size = 0;
    page_directory[33].unused_bit = 0;


    page_directory[33].available = 0;      


    //need to make sure the virtual address
    //page_directory[33] correspodns to virtual address 0x8400000


     //*screen_start = 0x8400000;


    vidmap_page[0].present=1;
    vidmap_page[0].user_supervisor=1;
    vidmap_page[0].read_write=1;
    vidmap_page[0].offset_31_12=(num>>12);
    //flush_tlb((int)page_directory);
}

/*
 * set_background_memory
 *   DESCRIPTION: Sets up background memory based on terminal num.
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Modifies the page directory entries for the process
 */
void set_background_memory(int terminal_num){
     //int pg_index = 0xB8000 >> 12;
     int target_address = (0xB8000+(terminal_num + 1)*4096) >> 12;
     video_memory_page[184].offset_31_12 = target_address;
}  
