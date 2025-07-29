// ------------------------------------------
//  Author: Prof. Taeweon Suh
//          Computer Science & Engineering
//          College of Informatics, Korea Univ.
//  Date:   March 29, 2020
// ------------------------------------------

#define csd_LED_ADDR 0x41200000

.extern csd_main

.global main

main:
 	ldr r0, =Input_data		// loads the address of first word in Input_data
 	mov r7, r0				// copies the address for next cycles
 	mov r2, #31				// count for 31 comparisons
 	mov r3, #31				// count for 31 cycles

loop:						// loop for comparison
	add r1, r0, #4			// r1 has the address of the next word of r0
	ldr r4, [r0]			// loads the word in address that is in r0 (first word)
 	ldr r5, [r1]			// loads the word in address that is in r1 (second word)

	cmp r4, r5				// compare r4, r5 and update flags (r4- r5)
	movgt r6, r4			// use r6 register for the tmep register in exchanging
	movgt r4, r5			// copy r5 to r4
	movgt r5, r6			// copy r6(original data in r4) to r5

	str r4, [r0]			// store data in r4 to address in r0
	str r5, [r1]			// store data in r4 to address in r0

	add r0, r0, #4			// increment r0 by 4, r0 now has the address of next word

 	subs r2, r2, #1			// decrement r2 by 1, one comparison has been finished
 	bne loop				// get back to the loop if not all 31 comparisons have been finished

	subs r3, r3, #1			// decrement r3 by 1, one cylce has been finished
	bne sorting				// get back to the sorting if not all 31 cycles have been finished

	ldr r0, =Input_data		// loads the address of first word in Input_data
	ldr r1, =Output_data	// loads the address to save first word in Output_data
	mov r2, #32				// count to check all 32 words

loop_update:				// loop for updating to output_data
	ldr r3, [r0], #4		// loads the word at address in r0 to r3, and r0 = r0 + 4
	str r3, [r1], #4		// stores the word in r3 to address in r1, and r1 = r1 + 4

	subs r2, r2, #1			// decrement r2 by 1, one copying is finished
	bne loop_update			// get back to loop_update if not all 32 words have been copied
	b forever				// if all 32 words are copied, move to forever

sorting:					// initialize for new cycle
	mov r0, r7				// copies the address of first word in Input_data for the new cycle
	mov r2, #31				// copies 31 in r2 register to track 31 comparisons for new cycle
	b loop					// get back to loop


 	bl csd_main				// get back to the caller

forever:
	nop						// do nothing
 	b forever				// get back to forever

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
