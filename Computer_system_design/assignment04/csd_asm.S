// ------------------------------------------
//  Author: Prof. Taeweon Suh
//          Computer Science & Engineering
//          College of Informatics, Korea Univ.
//  Date:   April 27, 2020
// ------------------------------------------

#include "uart_init.s"

.global main


.macro DEBUGGING
   sub sp, sp, #16                  // sp = sp - 16(space for sp, lr, pc, cpsr)
   stmfd sp!, {r0-r12}               // store values in r0-r12 to stack
   add sp, sp, #68                  // sp = sp + 68, sp pointing the previous location(to store sp, lr, pc, cpsr)
   mov r0, r13                     // copy r13 to r0
   mov r1, r14                     // copy r14 to r1
   mov r2, r15                     // copy r15 to r2
   sub r2, r2, #28                  // r2 = r2 - 28, because pc is now pointing 7 instructions away from the pc calling macro
   mrs r3, cpsr                  // copy cpsr to r3
   stmfd sp!, {r0-r3}               // store values in r0-r3 to stack

   sub sp, sp, #52                  // sp = sp - 52, sp is now pointing latest value stored(r0)
   mov r8, sp                     // r8 = sp, r8 is used to print out register values stored

   UART_init                     // Initialize UART
   bl UART_debug                  // print out debugging

   ldmfd sp!, {r0-r12}               // load the register values of r0-r12 before printing out debugging
   ldmfd sp!, {r14}               // discard the value(sp) beacause don't need to restore stack pointer
   add sp, sp, #8                  // move to address cpsr value is stored
   ldmfd sp!, {r14}               // load cpsr value to r14
   msr cpsr, r14                  // load cpsr value before debugging
   sub sp, sp, #12                  // move back to address of link register value stored
   ldmfd sp!, {r14}               // load link register before debugging
   add sp, sp, #8                  // return to sp before debugging

.endm


main:
    ldr r0, =Input_data      // loads the address of first word in Input_data
    mov r7, r0            // copies the address for next cycles
    mov r2, #31            // count for 31 comparisons
    mov r3, #31            // count for 31 cycles
    DEBUGGING            // Debug

loop:                  // loop for comparison
   add r1, r0, #4         // r1 has the address of the next word of r0
   ldr r4, [r0]         // loads the word in address that is in r0 (first word)
   ldr r5, [r1]         // loads the word in address that is in r1 (second word)
   DEBUGGING            // Debug

   cmp r4, r5            // compare r4, r5 and update flags (r4- r5)
   DEBUGGING            // Debug
   movgt r6, r4         // use r6 register for the tmep register in exchanging
   movgt r4, r5         // copy r5 to r4
   movgt r5, r6         // copy r6(original data in r4) to r5
   DEBUGGING            // Debug

   str r4, [r0]         // store data in r4 to address in r0
   str r5, [r1]         // store data in r4 to address in r0

   add r0, r0, #4         // increment r0 by 4, r0 now has the address of next word

   subs r2, r2, #1         // decrement r2 by 1, one comparison has been finished
   DEBUGGING            // Debug
   bne loop            // get back to the loop if not all 31 comparisons have been finished

   subs r3, r3, #1         // decrement r3 by 1, one cylce has been finished
   DEBUGGING            // Debug
   bne sorting            // get back to the sorting if not all 31 cycles have been finished

   ldr r0, =Input_data      // loads the address of first word in Input_data
   ldr r1, =Output_data   // loads the address to save first word in Output_data
   mov r2, #32            // count to check all 32 words
   DEBUGGING            // Debug

loop_update:            // loop for updating to output_data
   ldr r3, [r0], #4      // loads the word at address in r0 to r3, and r0 = r0 + 4
   str r3, [r1], #4      // stores the word in r3 to address in r1, and r1 = r1 + 4

   subs r2, r2, #1         // decrement r2 by 1, one copying is finished
   DEBUGGING            // Debug
   bne loop_update         // get back to loop_update if not all 32 words have been copied
   b forever            // if all 32 words are copied, move to forever

sorting:               // initialize for new cycle
   mov r0, r7            // copies the address of first word in Input_data for the new cycle
   mov r2, #31            // copies 31 in r2 register to track 31 comparisons for new cycle
   DEBUGGING            // Debug
   b loop               // get back to loop

forever:
   nop                  // do nothing
   b forever            // get back to forever



UART_debug:
   stmfd sp!, {r14}               // Store return location(lr) in stack
   bl UART_dashline_f                // Print First dashline
   bl UART_registers                // Print registers
   bl UART_dashline_e               // Print End dashline
   ldmfd sp!, {r14}               // Load return location from stack
   mov pc, lr                     // return to the caller

UART_dashline_f:
   ldr   r1, =dashline_f               // load address of first dashline
   ldr   r0, =uart_Channel_sts_reg0      // load uart status_register
   b TX_loop                     // print out the data in r1(dashline_f)

UART_registers:
   ldr r1, =registers               // load address of names of registers
   ldr r0, =uart_Channel_sts_reg0      // load uart status_register
   mov r7, #16                     // r7 = 16, print 16 registers(r0-r15)
   stmfd sp!, {r14}               // store return location(lr) to the stack

print_reg:
   bl TX_loop                     // print out the data in r1(next register name)
   ldr r9, [r8], #4               // load the register value stored in stack
   bl TX_register                  // print out the register value
   subs r7, r7, #1                  // decrement r7
   bne print_reg                  // if not all 16 registers are printed, go back to print_reg

   bl UART_cpsr                  // print cpsr

   ldmfd sp!, {r14}               // load return location from the stack
   mov pc, lr                     // return to the caller

UART_dashline_e:
   ldr r1, =dashline_e               // load the address of end dashline
   ldr r0, =uart_Channel_sts_reg0      // load the uart status register
   b TX_loop                     // print out the value in r1(dashline_e)

TX_loop:
   // ---------  Check to see if the Tx FIFO is empty ------------------------------
   ldr r2, [r0]      // read Channel Status Register
   and   r2, r2, #0x8   // read Transmit Buffer Empty bit(bit[3])
   cmp   r2, #0x8      // check if TxFIFO is empty and ready to receive new data
   bne   TX_loop         // if TxFIFO is NOT empty, keep checking until it is empty
   //------------------------------------------------------------------------------

   ldrb r3, [r1], #1               // load 1 byte in the address stored in r1
   ldr  r4, =uart_TX_RX_FIFO0         // load the address of TX FIFO
   strb r3, [r4]                  // fill the TxFIFO with the value in r3(1byte data to send)
   cmp  r3, #0x00                  // if r3 is 0x00
   bne  TX_loop                  // end the loop

   mov  pc, lr                     // return to the caller

TX_register:
   mov r6, #28                     // count for 28 bit shifts
   stmfd sp!, {r14}               // store the return location to stack

find_next_hex:
   // ---------  Check to see if the Tx FIFO is empty ------------------------------
   ldr r2, [r0]      // read Channel Status Register
   and   r2, r2, #0x8   // read Transmit Buffer Empty bit(bit[3])
   cmp   r2, #0x8      // check if TxFIFO is empty and ready to receive new data
   bne   find_next_hex   // if TxFIFO is NOT empty, keep checking until it is empty
   //------------------------------------------------------------------------------

   mov r5, r9, lsr r6               // shift the value in r9(value of a register) by r6 bits
   and r5, r5, #0xF               // take out the last 4 bit
   bl hex_to_ascii                  // change the last 4 bit(hex) into ascii

   ldr  r4, =uart_TX_RX_FIFO0         // load the address of Tx FIFO register
   strb r5, [r4]                  // fill the TxFIFO with the hex value in ascii form

   mov r5, r9                     // r5 = r9(original value of cpsr)
   sub r6, r6, #4                  // decrement r6 by 4(to get next 4bits)
   cmp r6, #-4                     // if r6 is not -4
   bne find_next_hex               // go back to the loop

   ldmfd sp!, {r14}               // load the return location from the stack
   mov  pc, lr                     // return to the caller

hex_to_ascii:
   cmp r5, #9                     // compare the value with 9
   addle r5, r5, #0x30               // lower and equal to 9 -> add 0x30 to make it to ascii
   addgt r5, r5, #0x37               // greater to 9 -> add 0x37 to make it to ascii
   mov pc, lr                     // return to the caller

UART_cpsr:
   stmfd sp!, {r14}               // store the return location to the stack
   ldr r1, =cpsr                  // load the address of cpsr to r1
   bl TX_loop                     // print out the value in r1(cpsr)

   ldr r9, [r8], #4               // load the register value at address in r8
   mov r5, #0xF0000000               // r5 = 0xF000_0000
   and r3, r9, r5                  // take out first 4 bits from r9

   cmp r3, r5                     // if first 4bits are 1111
   ldreq r1, =NZCV                  // load NZCV
   beq print_nzcv                  // print string
   cmp r3, #0xE0000000               // if first 4bits are 1110
   ldreq r1, =NZCv                  // load NZCv
   beq print_nzcv                  // print string
   cmp r3, #0xD0000000               // if first 4bits are 1101
   ldreq r1, =NZcV                  // load NZCv
   beq print_nzcv                  // print string
   cmp r3, #0xC0000000               // if first 4bits are 1100
   ldreq r1, =NZcv                  // load NZcv
   beq print_nzcv                  // print string
   cmp r3, #0xB0000000               // if first 4bits are 1011
   ldreq r1, =NzCV                  // load NzCV
   beq print_nzcv                  // print string
   cmp r3, #0xA0000000               // if first 4bits are 1010
   ldreq r1, =NzCv                  // load NzCv
   beq print_nzcv                  // print string
   cmp r3, #0x90000000               // if first 4bits are 1001
   ldreq r1, =NzcV                  // load NzcV
   beq print_nzcv                  // print string
   cmp r3, #0x80000000               // if first 4bits are 1000
   ldreq r1, =Nzcv                  // load Nzcv
   beq print_nzcv                  // print string
   cmp r3, #0x70000000               // if first 4bits are 0111
   ldreq r1, =nZCV                  // load nZCV
   beq print_nzcv                  // print string
   cmp r3, #0x60000000               // if first 4bits are 0110
   ldreq r1, =nZCv                  // load nZCv
   beq print_nzcv                  // print string
   cmp r3, #0x50000000               // if first 4bits are 0101
   ldreq r1, =nZcV                  // load nZcV
   beq print_nzcv                  // print string
   cmp r3, #0x40000000               // if first 4bits are 0100
   ldreq r1, =nZcv                  // load nZcv
   beq print_nzcv                  // print string
   cmp r3, #0x30000000               // if first 4bits are 0011
   ldreq r1, =nzCV                  // load nzCV
   beq print_nzcv                  // print string
   cmp r3, #0x20000000               // if first 4bits are 0010
   ldreq r1, =nzCv                  // load nzCv
   beq print_nzcv                  // print string
   cmp r3, #0x10000000               // if first 4bits are 0001
   ldreq r1, =nzcV                  // load nzcV
   beq print_nzcv                  // print string
   cmp r3, #0x0                  // if first 4bits are 0000
   ldreq r1, =nzcv                  // load nzcv
   beq print_nzcv                  // print string

print_nzcv:
   bl TX_loop

   mov r5, #0x00000100               // 0b1_0000_0000
   and r3, r9, r5                  // A bit
   cmp r3, r5                     // if A bit is 1
   ldreq r1, =A                  // load the address of A
   bleq TX_loop                  // print out A

   mov r5, #0x00000080               // 0b1000_0000
   and r3, r9, r5                  // I bit
   cmp r3, r5                     // if I bit is 1
   ldreq r1, =I                  // load the address of I
   bleq TX_loop                  // print out I

   mov r5, #0x00000040               // 0b0100_0000
   and r3, r9, r5                  // F bit
   cmp r3, r5                     // if F bit is 1
   ldreq r1, =F                  // load the address of F
   bleq TX_loop                  // print out F

   ldr r7, =0x01000020               // 0b0000_0001_0000_0000_0000_0000_0010_0000
   mov r5, r7                     // r5 = #0x01000020
   and r3, r9, r5                  // J and T bit
   cmp r3, r5                     // if J and T bits are 1
   ldreq r1, =ThumbEE               // current ISA is ThumbEE
   beq print_isa                  // print current ISA
   cmp r3, #0x01000000               // if J bit is 1
   ldreq r1, =Jazelle               // current ISA is Jazelle
   beq print_isa                  // print current ISA
   cmp r3, #0x00000010               // if T bit is 1
   ldreq r1, =Thumb               // current ISA is Thumb
   beq print_isa                  // print current ISA
   cmp r3, #0x00000000               // if J and T bits are 0
   ldreq r1, =ARM                  // current ISA is ARM
   beq print_isa                  // print current ISA

print_isa:
   bl TX_loop                     // print the string(ISA)

   eor r3, r3, r3                  // initiallize r3(no reason)
   ldr r1, =m                     // load the string: mode
   bl TX_loop                     // print the string

   eor r3, r3, r3                  // initiallize r3(no reason)
   ldr r1, =curmode               // load the string: , current mode =
   bl TX_loop                     // print the string

   mov r5, #0x0000001F               // 0b0000_0000_0000_0000_0000_0000_0001_1111
   and r3, r9, r5                  // M[4:0]
   cmp r3, r5                     // if M[4:0] is 11111
   ldreq r1, =SYS                  // current mode is SYS
   beq print_mode                  // print string(mode)
   cmp r3, #0x0000001B               // if M[4:0] is 11011
   ldreq r1, =UND                  // current mode is UND
   beq print_mode                  // print string(mode)
   cmp r3, #0x0000001A               // if M[4:0] is 11010
   ldreq r1, =HYP                  // current mode is HYP
   beq print_mode                  // print string(mode)
   cmp r3, #0x00000017               // if M[4:0] is 10111
   ldreq r1, =ABT                  // current mode is ABT
   beq print_mode                  // print string(mode)
   cmp r3, #0x00000016               // if M[4:0] is 10110
   ldreq r1, =MON                  // current mode is MON
   beq print_mode                  // print string(mode)
   cmp r3, #0x00000013               // if M[4:0] is 10011
   ldreq r1, =SVC                  // current mode is SVC
   beq print_mode                  // print string(mode)
   cmp r3, #0x00000012               // if M[4:0] is 10010
   ldreq r1, =IRQ                  // current mode is IRQ
   beq print_mode                  // print string(mode)
   cmp r3, #0x00000011               // if M[4:0] is 10001
   ldreq r1, =FIQ                  // current mode is FIQ
   beq print_mode                  // print string(mode)
   cmp r3, #0x00000010               // if M[4:0] is 10000
   ldreq r1, =USR                  // current mode is USR
   beq print_mode                  // print string(mode)

print_mode:
   bl TX_loop                     // print the string

   ldr r1, =brkt_start               // load start of the bracket: ( 0x=
   bl TX_loop                     // print the string

   mov r1, r9                     // load the value of the cpsr register(32-bit)
   bl TX_register                  // print the value in ascii

   ldr r1, =brkt_end               // load end of the bracket: )
   bl TX_loop                     // print the string


   ldmfd sp!, {r14}               // load the return location from the stack
   mov pc, lr                     // return to the caller

   .data
dashline_f:
   .ascii "\r\n--------------------------------------------------------------"
   .byte 0x00

registers:
   .ascii "\r\nr0 = 0x"
   .byte 0x00
   .ascii ", r1 = 0x"
   .byte 0x00
   .ascii ", r2 = 0x"
   .byte 0x00
   .ascii ", r3 = 0x"
   .byte 0x00
   .ascii "\r\nr4 = 0x"
   .byte 0x00
   .ascii ", r5 = 0x"
   .byte 0x00
   .ascii ", r6 = 0x"
   .byte 0x00
   .ascii ", r7 = 0x"
   .byte 0x00
   .ascii "\r\nr8 = 0x"
   .byte 0x00
   .ascii ", r9 = 0x"
   .byte 0x00
   .ascii ", r10= 0x"
   .byte 0x00
   .ascii ", r11= 0x"
   .byte 0x00
   .ascii "\r\nr12= 0x"
   .byte 0x00
   .ascii ", r13= 0x"
   .byte 0x00
   .ascii ", r14= 0x"
   .byte 0x00
   .ascii ", r15= 0x"
   .byte 0x00

cpsr:
   .ascii "\r\ncpsr = "
   .byte 0x00

NZCV:
   .ascii "NZCV, "
   .byte 0x00

NZCv:
   .ascii "NZCv, "
   .byte 0x00

NZcV:
   .ascii "NzcV, "
    .byte 0x00

NZcv:
   .ascii "NZcv, "
    .byte 0x00

NzCV:
   .ascii "NzCV, "
    .byte 0x00

NzCv:
   .ascii "NzCv, "
    .byte 0x00

NzcV:
   .ascii "NzcV, "
    .byte 0x00

Nzcv:
    .ascii "Nzcv, "
    .byte 0x00

nZCV:
    .ascii "nZCV, "
    .byte 0x00

nZCv:
    .ascii "nZCv, "
    .byte 0x00

nZcV:
    .ascii "nZcV, "
    .byte 0x00

nZcv:
    .ascii "nZcv, "
    .byte 0x00

nzCV:
    .ascii "nzCV, "
    .byte 0x00

nzCv:
    .ascii "nzCv, "
    .byte 0x00

nzcV:
    .ascii "nzcV, "
    .byte 0x00

nzcv:
    .ascii "nzcv, "
    .byte 0x00

A:
   .ascii "A"
   .byte 0x00
I:
   .ascii "I"
   .byte 0x00
F:
   .ascii "F"
   .byte 0x00

ARM:
   .ascii ", ARM "
   .byte 0x00

Thumb:
   .ascii ", Thumb "
   .byte 0x00

Jazelle:
   .ascii ", Jazelle "
   .byte 0x00

ThumbEE:
   .ascii ", ThumbEE "
   .byte 0x00

USR:
   .ascii "USR "
   .byte 0x00

FIQ:
   .ascii "FIQ "
   .byte 0x00

IRQ:
   .ascii "IRQ "
   .byte 0x00

SVC:
   .ascii "SVC "
   .byte 0x00

MON:
   .ascii "MON "
   .byte 0x00

ABT:
   .ascii "ABT "
   .byte 0x00

HYP:
   .ascii "HYP "
   .byte 0x00

UND:
   .ascii "UND "
   .byte 0x00

SYS:
   .ascii "SYS "
   .byte 0x00

brkt_start:
   .ascii "(0x="
   .byte 0x00

brkt_end:
   .ascii ")"
   .byte 0x00

m:
   .ascii "mode"
   .byte 0x00

curmode:
   .ascii ", current mode = "
   .byte 0x00

dashline_e:
   .ascii "\r\n--------------------------------------------------------------"
   .byte 0x00

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
