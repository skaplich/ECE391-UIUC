#include "fileSystem.h"

/* 
 * open_file
 *   DESCRIPTION: Opens a file with the specified filename and reads its
 *                corresponding directory entry. Sets boot_temp and 
 *                dentry_temp to the relevant values.
 *   INPUTS: fname -- pointer to the filename string
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success
 *   SIDE EFFECTS: modifies boot_temp and dentry_temp
 */
int dir_index = 0;

int32_t open_file(const uint8_t* fname)
{
    cli();
    if(strlen((int8_t*)fname) > 32){
        sti();
        return -1;
    }

    boot_temp = (boot_block*)file_start; 
    inode_temp = (inode_block *)(boot_temp + 1);
            //file start refers to the address of the boot block defined elsewhere in kernel.c
    if(read_dentry_by_name(fname, &dentry_temp) == -1){
        sti();
        return -1;
    }
    file_size = (inode_temp + dentry_temp.inode_num)->file_length;
    sti();
    return 0;
}

/* 
 * close_file
 *   DESCRIPTION: Closes the currently opened file by resetting boot_temp.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success
 *   SIDE EFFECTS: resets boot_temp
 */
int32_t close_file(uint32_t fd)
{
    //boot_temp = NULL;    // undo the open file data
    return 0;
}


/* 
 * write_file
 *   DESCRIPTION: Does nothing as writing is not supported.
 *   RETURN VALUE: -1
 */
int32_t write_file(uint32_t fd, const void* buf, uint32_t length)
{
    return -1;
}

/* 
 * read_file
 *   DESCRIPTION: Reads data from the currently opened file into a buffer.
 *   INPUTS: buf -- pointer to the buffer where data will be stored
 *           length -- length of data to be read in bytes
 *   OUTPUTS: none
 *   RETURN VALUE: number of bytes read on success, -1 on failure
 *   SIDE EFFECTS: none
 */
int32_t read_file(uint32_t fd, uint8_t* buf, uint32_t length)
{
    
    pcb_t * pcb_temp = (pcb_t*)(get_pcb_using_pid(current_pid));

    //fd_t * file_array = pcb_temp->fda;
    uint32_t nbytes;
    if(boot_temp != NULL){ //make sure struct is set
        nbytes = read_data(pcb_temp->fda[fd].inode_num, pcb_temp->fda[fd].file_pos, buf, length); //call read_data
        if(nbytes == -1){
            return -1;
        }
    }
    else{
        return -1;
    }
    return nbytes;

}


/* 
 * open_directory
 *   DESCRIPTION: Opens a directory and sets boot_temp accordingly.
 *   INPUTS: fname -- pointer to the directory name string
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success
 *   SIDE EFFECTS: modifies boot_temp
 */
int32_t open_directory(const uint8_t* fname)
{
   boot_temp = (boot_block*)file_start; // set struct to file start
    return 0;
}


/* 
 * close_directory
 *   DESCRIPTION: Closes the currently opened directory by resetting boot_temp.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success
 *   SIDE EFFECTS: resets boot_temp
 */
int32_t close_directory(uint32_t fd)
{
    //boot_temp = NULL;
    return 0;
}

/* 
 * write_directory
 *   DESCRIPTION: Does nothing as writing is not supported.
 *   RETURN VALUE: -1
 */
int32_t write_directory(uint32_t fd, const void* buf, uint32_t length) //does nothing, returns -1
{
    return -1;
}   

/* 
 * read_directory
 *   DESCRIPTION: Reads directory entries into a buffer.
 *   INPUTS: buf -- pointer to the buffer where directory entries will be stored
 *           length -- length of data to be read
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success
 *   SIDE EFFECTS: modifies buf
 */
int32_t read_directory(uint32_t fd,uint8_t* buf, uint32_t length)     //calls read data
{
    cli();
        // READ DIR NEEDS TO WORK WITH BYTE limits

    int size = boot_temp->num_dir_entries;   // number of files in the directory
    if(dir_index == size){
        dir_index = 0;
        sti();
        return 0;
    }
    
    int j;
    
    int max = (int)strlen((int8_t *)boot_temp->dir_entries[dir_index].file_name);   // length of the current file name we will parse through
    if(max > length){
        max = length;
    }
   
    for(j = 0; j < max; j++) 
    {
        *buf = boot_temp->dir_entries[dir_index].file_name[j];   //writting one character into the buffer
        buf++;
    }
    
    dir_index++;
    sti();
    return max;


    
   
    
}



/* 
 * read_dentry_by_name
 *   DESCRIPTION: Finds a directory entry by name and populates the provided
 *                dentry structure with its information.
 *   INPUTS: fname -- pointer to the filename string
 *           dentry -- pointer to the dentry structure to be populated
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: modifies dentry
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_entry* dentry)
{
    uint32_t size = boot_temp->num_dir_entries;       // number of files and directories in the file system
    int i;
    for(i = 0; i < size; i++){
        if(strncmp((int8_t *)fname,(int8_t *)(boot_temp->dir_entries)[i].file_name, 32) == 0){  //copy over all 32 bytes, check if the file name corresponds to this dentry struct
            
            strcpy((int8_t*)dentry->file_name, (int8_t*)(boot_temp->dir_entries)[i].file_name);   // copy contents of dentry name to the dentry struct passed in
            dentry->file_type = (boot_temp->dir_entries)[i].file_type;  // copy additional information about the file 
            dentry->inode_num = (boot_temp->dir_entries)[i].inode_num;
            return 0;
        }
    }
    return -1;
}


/* 
 * read_dentry_by_index
 *   DESCRIPTION: Finds a directory entry by index and populates the provided
 *                dentry structure with its information.
 *   INPUTS: index -- index of the directory entry to read
 *           dentry -- pointer to the dentry structure to be populated
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: modifies dentry
 */
int32_t read_dentry_by_index(uint32_t index, dentry_entry* dentry)
{
    uint32_t size = boot_temp->num_dir_entries;
    if(index < 0 || index > size-1){ //index out of bounds    (-1 because one file is reserved for root directory)
        return -1; 
    }
    strcpy((int8_t*)dentry->file_name, (int8_t*)(boot_temp->dir_entries)[index].file_name); // copy over file name to the new dentry
    dentry->file_type = (boot_temp->dir_entries)[index].file_type; // copy over additional information
    dentry->inode_num = (boot_temp->dir_entries)[index].inode_num;
    return 0;
   
}


/* 
 * read_data
 *   DESCRIPTION: Reads data blocks associated with an inode into a buffer.
 *   INPUTS: inode -- index of the inode to read data from
 *           offset -- starting offset within the inode's data blocks
 *           buf -- pointer to the buffer where data will be stored
 *           length -- length of data to be read
 *   OUTPUTS: none
 *   RETURN VALUE: number of bytes read on success, -1 on failure
 *   SIDE EFFECTS: modifies buf
 */

int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
    //memset(buf,0,length);
     int bytes_read = 0;    
    int num_inodes = boot_temp->inodes; // number of inodes in the data block
    int num_data_blocks = boot_temp->data_blocks; // number of data blocks used to store all files in file system
    inode_block * inode_start = (inode_block *)(boot_temp + 1); // start of the address of the inodes 
   
    if(inode < 0 || inode > num_inodes-1){
        return -1; //inode out of bound
    }
    if(length == 0)
    {
        return bytes_read;
    }
    inode_block * inode_temp = (inode_start + inode); //desired inode
    int block_offset = offset/BLOCKS_SIZE;
    int byte_offset = offset%BLOCKS_SIZE;
    int i,j;
    
    for(i = block_offset; i < num_data_blocks; i++){   // go through all of the data blocks in the whole file system (not efficient)
        uint8_t * curr_block_ptr = (uint8_t *)(inode_start + (num_inodes + inode_temp->data_blocks[i]));  // save the address of the current data block

        if(i == block_offset)
        {
            curr_block_ptr += byte_offset;
            for(j = byte_offset; j < BLOCKS_SIZE; j++){ // read through all of the bytes in the data block (4096 bytes) 
                if(bytes_read == length){ // if the buffer is full
                    return length;
                }
                if((bytes_read + offset) == file_size)
                {
                    return bytes_read;
                }
            
                buf[bytes_read] = *(curr_block_ptr); // write to the buffer
                bytes_read++;
                curr_block_ptr += (uint8_t)1; // increment the pointer of the current byte in the data block 
            }        
        }

        else 
        { 
            for(j = 0; j < BLOCKS_SIZE; j++){ // read through all of the bytes in the data block (4096 bytes) 
                if(bytes_read == length){ // if the buffer is full
                    return length;
                }
                if((bytes_read + offset) == file_size)
                {
                    return bytes_read;
                }
                buf[bytes_read] = (uint8_t)*(curr_block_ptr); // write to the buffer
                bytes_read++;
                curr_block_ptr += (uint8_t)1; // increment the pointer of the current byte in the data block 
            }
        }

    }
    return -1;
    
}




/* 
 * flush_tlb
 *   DESCRIPTION: Flushes the translation lookaside buffer (TLB).
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Flushes TLB
 */
void flush_tlb(){
    asm volatile(
        "movl %%cr3,%%eax     ;"
        "movl %%eax,%%cr3     ;"

        : : : "eax", "cc" 
    );
}

/* 
 * get_pcb_using_pid
 *   DESCRIPTION: Retrieves the address of the process control block (PCB) using the process ID (PID).
 *   INPUTS: pid - process ID
 *   OUTPUTS: none
 *   RETURN VALUE: Address of the PCB
 *   SIDE EFFECTS: none
 */
int32_t get_pcb_using_pid(int32_t pid)
{
    return 0x800000 - (0x2000 * (pid + 1)); // 8MiB - 4kiB * pid
}
