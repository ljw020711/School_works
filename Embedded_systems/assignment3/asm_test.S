
/*
 * Assembler1.s
 *
 * Created: 7/27/2023 10:27:01 AM
 *  Author: suhtw
 */ 

 .syntax unified
  
 .section .text
 .globl lab_asm_port_test
 .globl lab_asm_func_test
 
 // Prof. Suh: 
 // if directive is removed (.thumb, .thumb_func),
 // exception occurs because blx instruction is branching to Arm code..
 // 
 // Note: Cortex-M0+ only supports thumb instructions

 .thumb
 .thumb_func

 lab_asm_port_test:
   ldr r0, =0x41004446		// Pin Configuration Register: PA06
   ldr r1, =0x41004447		// Pin Configuration Register: PA07
   ldr  r2, =0x0			// bit 1 = 0, bit 0 = 0
   strb r2, [r0]			// store 0x0 into PINCFG06 register
   strb r2, [r1]			// store 0x0 into PINCFG07 register

   ldr r0, =0x41004400		// Direction Register
   ldr r1, =0x000000c0		// bit 6 = 1, bit 7 = 1, bit 8 = 0, bit 9 = 0 (6, 7 output 8, 9 input)
   str r1, [r0]				// store 0xc0 into DIR register

   ldr r0, =0x41004448		// Pin Configuration Register: PA08
   ldr r1, =0x41004449		// Pin Configuration Register: PA09
   ldr  r2, =0x2			// bit 1 = 1, bit 0 = 0
   strb r2, [r0]			// store 0x2 into PINCFG08 register
   strb r2, [r1]			// store 0x2 into PINCFG09 register

   ldr r0, =0x41004420		// IN Register
   ldr r4, =0x41004410		// OUT Register

   movs r5, #0x1			// copy 0x1 into r5, used to pop out lsb of IN register

 loop:
   ldr r1, [r0]				// Load Input in r1 register (1 = clicked, 0 = not clicked)

   lsrs r2, r1, #8			// Logical Shift Right 8 bit to put the input value of PA08 to LSB
   ands r2, r2, r5			// pop out lsb of IN register(input value of PA08) to r2 register
   lsls r2, r2, #7			// Logical Shift Left 7 bit to put the input value of PA08 to bit of PA07

   lsrs r3, r1, #9			// Logical Shift Right 9 bit to put the input value of PA09 to LSB
   ands r3, r3, r5			// pop out lsb of IN register(input value of PA09) to r3 register
   lsls r3, r3, #6			// Logical Shift Left 6 bit to put the input value of PA09 to bit of PA06

   orrs r2, r2, r3			// OR r2, r3 to merge the output bits of PA06 and PA07
   str r2, [r4]				// store the result to OUT register
   b loop					// branch to loop so that to check button is clicked or not

   bx lr					// return to the caller


// Prof. Suh: 
// * Try to execute the code w/o '.thumb .thumb_func' 
// * --> Exception occurs due to 'bx lr'
//  (trying to change ISA to Arm, but it only supports Thumb)

.thumb
.thumb_func

lab_asm_func_test:
   mrs r0, psr
   mrs r1, epsr
   msr epsr, r1
   add r9, r0
   nop

   movs r0, #1
   movs r1, #2
   add r0, r0, r1
   
   bx lr
