
/*
 * Assembler1.s
 *
 * Created: 7/27/2023 10:27:01 AM
 *  Author: suhtw
 */ 

 .syntax unified
  
 .section .text
 .globl asm_test
 

 .thumb
 .thumb_func

 asm_test:
   ldr r0, =Input_data      // loads the address of Input_data
   movs r7, r0				// copys the address of Input_data for getting back to the first word
   movs r6, #31				// copy 31 into r6 register, 31 loops total(bubble sort)
   movs r5, #31				// copy 31 into r5 register, 31 comparisons in one cycle

 loop:						// loop for comparing two words
   adds r1, r0, #4			// r1 register has the address of next word

   ldr r2, [r0]				// loads current word in r2 register
   ldr r3, [r1]				// loads the next word in r3 register

   cmp r2, r3				// compare to words
   ble continue				// if r2 <= r3, then no exchange. Exchange only when r2 > r3

   movs r4, r3				// use r4 as a temp register for swapping, copy r3 into r4
   movs r3, r2				// copy r2 into r3
   movs r2, r4				// copy r4 into r2 (previous r3 value)

 continue:					// continue next code
   str r2, [r0]				// store data in r2 into address in r0 (goes front)
   str r3, [r1]				// store data in r3 into address in r1 (goes back)

   adds r0, r0, #4			// increment r0 by 4: r0 has the address of next word

   subs r5, r5, #1			// decrement r5 by 1: one comparision finished
   bne loop					// go for this loop until all comparison(31) is finished

   subs r6, r6, #1			// decrement r6 by 1: one cycle (comparing all 31 words) finished
   bne sorting				// go to sorting if it didn't went over for 31 cycles

   b update					// if all sorting is finished, go to update

 sorting:					// sorting: go back to the first word if all cycles are not finished
   movs r0, r7				// r0 = the address of the first word
   movs r5, #31				// copy 31 into r5, compare 31 times for each cycle
   b loop					// go back to loop for comparing

 update:					// copy all 32 words into Output_data address
   ldr r0, =Input_data      // loads the address of Input_data 
   ldr r1, =Output_data		// loads the address of Output_data
   movs r2, #32				// copy 32 into r2, count for all 32 words

 loop_copy:					// loop for copying to Output_data address
   ldr r3, [r0]				// load the word in from address in r0 into r3
   str r3, [r1]				// store the word in r3 into address in r1

   adds r0, r0, #4			// increment r0 by 4, address of next word
   adds r1, r1, #4			// increment r1 by 4, address of next place to store

   subs r2, r2, #1			// decrement r2 by 1, tracking how many words left to copy
   bne loop_copy			// if r2 isn't 0, then keep copying next word
   b end					// branch to end if all copying is finished

 end:						// indicating end (Not used)



.data
.align 4

Input_data: 
   .word 2, 0, -7, -1, 3, 8, -4, 10
   .word -9, -16, 15, 13, 1, 4, -3, 14
   .word -8, -10, -15, 6, -13, -5, 9, 12
   .word -11, -14, -6, 11, 5, 7, -2, -12

Output_data: 
   .word 0, 0, 0, 0, 0, 0, 0, 0
   .word 0, 0, 0, 0, 0, 0, 0, 0
   .word 0, 0, 0, 0, 0, 0, 0, 0
   .word 0, 0, 0, 0, 0, 0, 0, 0