// #include "tests.h"
// #include "x86_desc.h"
// #include "lib.h"
// #include "invalidOpcode.h"
// #include "rtc.h"
// #include "keyboard.h"
// #include "terminaldriver.h"
// #include "fileSystem.h"


// #define PASS 1
// #define FAIL 0


// // //rtc testvariable
// // #define testrtc 1

// /* format these macros as you see fit */
// #define TEST_HEADER     
//     printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
// #define TEST_OUTPUT(name, result)   
//     printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");


// static inline void assertion_failure(){
//     /* Use exception #15 for assertions, otherwise
//        reserved by Intel */
//     asm volatile("int $15");
// }




// /* Checkpoint 1 tests */


// /* IDT Test - Example
//  *
//  * Asserts that first 10 IDT entries are not NULL
//  * Inputs: None
//  * Outputs: PASS/FAIL
//  * Side Effects: None
//  * Coverage: Load IDT, IDT definition
//  * Files: x86_desc.h/S
//  */
// int idt_test(){
//     TEST_HEADER;


//     int i;
//     int result = PASS;
//     for (i = 0; i < 10; ++i){
//      //if(i == 0x21 || i == 0x28 || i == 0x80 || (i >= 0x00 && i <= 0x13)){
//      if ((idt[i].offset_15_00 == NULL) &&
//          (idt[i].offset_31_16 == NULL)){
//          assertion_failure();
//          result = FAIL;
//      }
//     //}
//     }


//     return result;
// }


// /*Divide by 0 error , checks if divide by 0 is leads to the exception
// inputs:non
// Outputs: divide by 0 exception
// Side Effects: goes into exception handler
// */


// // int divideByZero(){
// //     int a = 5 / 0; //divides by 0
// //     return a;
// // }




// /*Invalid Opcode test, checks for invalid opcode
// inputs:uses asm code from InvalidOpcode.S to send invalid opcode
// Outputs: invalid opcode error
// Side Effects: goes into exception handler
// */


// int invalidOpcodeS(){
// 	opcodeTest(); // calls asm code

// 	return 1;
	

// }

// int pagingTest(){
//     int * a = NULL;
//     return *a; //deref of a null
// }


// /*Invalid segment test, checks for invalid opcode
// inputs:uses asm code from InvalidOpcode.S to do syscall without settings regs
// Outputs: segment error test
// Side Effects: goes into exception handler
// */

// int segError(){
// 	segmentErrorCall(); // calls asm code

// 	return 1;
// }





// // add more tests here


// /* Checkpoint 2 tests */

// /*Terminal test, checks open, close, read, write terminal driver
// inputs:none
// Outputs: terminal allowing reading and writing
// Side Effects: opens and closes terminals 
// */
// void terminal_test()
// {
//     TEST_HEADER;
//     //int result = PASS;
//     clear_screen();//clear screen
//     int temp;
//     char buf[128];//buffer to hold chars
//     openterminal();//call open
//     writeterminal((uint8_t*)"open and write work\n", 20);//open works
//     temp = readterminal(buf, 127);//read from temrinal
//     writeterminal(buf, temp);//write, but will cut due to close next

//     closeterminal();//close terminal
//     openterminal();//open new terminal
//     writeterminal((uint8_t*)"read and close work\n", 20);//read, close, write works
//     temp = readterminal(buf, 127);//read from terminal
//     writeterminal(buf, temp);//write

//     closeterminal();//close terminal
//     openterminal();//open new terminal 
//     writeterminal((uint8_t*)"-YAY-TERMINAL-\n", 15); //final terminal
//     while(1){
//         temp = readterminal(buf, 127);        //read terminal
//         writeterminal(buf, temp);//write terminal
//     }
//     closeterminal();
// }

// /*rtc test, checks open, close, read, write rtc driver
// inputs:none
// Outputs: rtc allowing reading and writing changing freq
// Side Effects: opens and closes rtc 
// */
// void rtc_test()
// {

//     clear_screen();
//         int j;
//         int freq;
//         int32_t retval = 0;
//         retval += openrtc(); //testing openrtc
//         freq=64;
//         rtcfreq(freq);//set freq of rtc
//         for(j = 0; j <= 1024; j++) 
//         {
//                 retval += writertc(freq); //test write rtc
//                 //printf("Testing: %d Hz\n", i);
//                 //printf("Testing: RTC\n");
//                 retval+=readrtc();//test read rtc
//             }
//             closertc();//test closertc
//             // if(512>j&&j>256)
//             // {
//             //     i=32;
//             //     rtcfreq(i);
//             //     retval += writertc(i);
//             //     printf("Testing: %d Hz\n", i);
//             //     retval+=readrtc();

//             // }
//             // if(512<j&&j<(256+512))
//             // {
//             //     i=256;
//             //     rtcfreq(i);
//             //     retval += writertc(i);
//             //     printf("Testing: %d Hz\n", i);
//             //     retval+=readrtc();

//             // }
//             // if(j<(256+512))
//             // {
//             //      i=1024;
//             //     rtcfreq(i);
//             //     retval += writertc(i);
//             //     //printf("Testing: %d Hz\n", i);    
//             //     retval+=readrtc();
           
//             // }
//             //test_interrupts();
        
// }

// // /*rtc test, checks open, close, read, write rtc driver
// // inputs:none
// // Outputs: rtc allowing reading and writing changing freq
// // Side Effects: opens and closes rtc 
// // */
// // void rtc_test()
// // {
// //     int i;
// //     openrtc();
// //     rtcfreq(4);
// //     test_interrupts();
// //     for(i=0; i<100000; i++)
// //     {
// //            rtcfreq(1024);
// // 	       test_interrupts();
// //     }
// //     closertc();

// //     openrtc();
// //     rtcfreq(4);
// //     test_interrupts();
// //     for(i=0; i<100000; i++)
// //     {
// //            rtcfreq(64);
// // 	       test_interrupts();
// //     }
// //     closertc();

// //     openrtc();
// //     rtcfreq(4);
// //     test_interrupts();
// //     for(i=0; i<100000; i++)
// //     {
// //            rtcfreq(2);
// // 	       test_interrupts();
// //     }
// //     closertc();

// //     openrtc();
// //     rtcfreq(4);
// //     test_interrupts();
// //     for(i=0; i<100000; i++)
// //     {
// //            rtcfreq(16);
// // 	       test_interrupts();
// //     }
// //     closertc();
// // }

// /* Checkpoint 3 tests */
// /* Checkpoint 4 tests */
// /* Checkpoint 5 tests */


// //Testing the Directory read function using an arbitrary directory name, since the current file system is "flat"
// int DirTest(){
//     int i;
//     clear();
//      //openterminal();
//     printf("\n"); // formatting for QEMU
//     printf("\n");
//     uint8_t testname[32] = "frame0.txt"; // select the file name //32 bit max name
//     uint8_t buf [BLOCKS_SIZE];
//     memset(buf, ' ', sizeof(buf)); //set the buffer contents to nothing, prevents random garbage from being displayed
    
//     open_directory((uint8_t*)&testname); // open the directory
//     read_directory(buf, BLOCKS_SIZE); // read the directory contents
//     for(i = 0; i < BLOCKS_SIZE; i++)
//     {
//             originalputc(buf[i]); // display the buffer
//     }
//     close_directory(); //close the directory 
//     return 1;
// }

// // Testing the read file function on the frame0.txt file, change the file name below to read a different file.
// int FileReadTest(){
//     int i;
//     clear();
//     //openterminal();
//     printf("\n"); // formatting because QEMU is weird
//     printf("\n");
//     //int filesize = getfilesize();
//     int filesize = BLOCKS_SIZE;
//     uint8_t buf [BLOCKS_SIZE]; // create a buffer that is 4096 bytes long (one data block)
//     uint8_t testname[32] = "frame0.txt"; // select the file name we want
    
//     open_file((uint8_t*)&testname); // init/open the file 
//     int bytes_returned = read_file(buf, BLOCKS_SIZE);  //read file contents and write to the buffer
    
//      for(i = 0; i < filesize; i++){   
//         if(buf[i] != NULL){
//              originalputc(buf[i]); // print char by char onto the screen
//          }
//     }

//     close_file(); // close the file (destroy the init variables)
//     if(bytes_returned == BLOCKS_SIZE){ // check if number of bytes is correct
//         return 1;
//     }
//     else{
//         return 0;
//     }
    
   
    
// }





// /* Test suite entry point */
// void launch_tests(){
//     //TEST_OUTPUT("idt_test", idt_test());
//     //TEST_OUTPUT("paging_test", pagingTest());


//      //TEST_OUTPUT("Divide by 0 test", divideByZero());

// 	  //TEST_OUTPUT("Invalid Opcode", invalidOpcodeS());

// 	//TEST_OUTPUT("Seg Fault Test", segError());
//     //TEST_OUTPUT("rtc", rtc_test());
//     //rtc_test();
//     rtc_test();
//     //terminal_test();
    
//     //TEST_OUTPUT("file_Read_test", FileReadTest());
//     //TEST_OUTPUT("dir_test", DirTest());
//     //terminal_test();
//     // launch your tests here
// }

