// ------------------------------------------
//  Author: Prof. Taeweon Suh
//          Computer Science & Engineering
//          College of Informatics, Korea Univ.
//  Date:   May 06, 2020
// ------------------------------------------

#include "csd_zync_peripherals.h"

.align 5

csd_vector_table:
   b .
   b .
   b .
   b .
   b .
   b .
   b csd_IRQ_ISR
   b .

.global main
main:

   // Disable interrupt: CPSR'I = 1
   cpsID i

   cps #0x12                // IRQ mode
   ldr   r13,=irq_stack_top // Stack pointer setup for IRQ mode

   cps #0x13                // supervisor mode
   ldr   r13,=svc_stack_top // Stack pointer setup for SVC mode

   cps #0x11                // FIQ mode
   ldr   r13,=fiq_stack_top // Stack pointer setup for FIQ mode

   cps #0x1F                // SYS mode

   // Set VBAR (Vector Base Address Register) to my vector table
   ldr     r0, =csd_vector_table
   mcr     p15, 0, r0, c12, c0, 0
   dsb
   isb

   // Enable interrupt: CPSR'I = 0
   cpsIE i

// ---------------------------
// Generic Interrupt Controller (GIC) setup - Begin
// ---------------------------

   // CPU Interface ID Register
   ldr r0, =GICC_IIDR
   ldr r3, [r0]

   // CPU Controller Type Register
   ldr r0, =GICD_TYPER
   ldr r3, [r0]

   // CPU Binary Pointer Register
   ldr r0, =GICC_BPR
   ldr r3, [r0]

   // Distributor Control Register
   ldr r0, =GICD_CTLR
   ldr r1, [r0]
   mov r2, #1                   // Enable
   orr r1, r1, r2
   str r1, [r0]
   ldr r3, [r0]

    // Interrupt Set-Enable Register 0
   ldr r0, =GICD_ISENABLER0      // load Enable register
   ldr r1, [r0]               // load the value
   mov r2, #1 << 29               // Enable #29 (Private Timer)
   orr r1, r1, r2               // set bits for 29
   str r1, [r0]               // store value
   ldr r3, [r0]               // for checking

   // According to specifications,
   // Cortex-A9 supports 5-bit version of priority format [7:3] in secure world
   // ( 0 -> 8 -> 16 -> 24...)

   // Interrupt Priority Register #7
   ldr r0, =GICD_PRIOR7         // 29/4 = 7
   ldr r1, [r0]               // load
   mov r2, #0x10 << 8             // Priority 16 for ID# 29 (Private Timer)
   orr r1, r1, r2               // set bits for ID# 29
   str r1, [r0]               // store value
   ldr r3, [r0]               // for checking

   // CPU Interface Control Register
   ldr r0, =GICC_CTLR
   ldr r1, [r0]
   mov r2, #1                    // Enable
   orr r1, r1, r2
   str r1, [r0]
   ldr r3, [r0]

    // CPU Interface Interrupt Priority Mask Register
   ldr r0, =GICC_PMR            // load Interruput Priority Mask Register
   ldr r1, [r0]               // get the value in the register
   mov r2, #0xFF                 // Lowest (accpet all interrupts)
   orr r1, r1, r2               // set bit in register value with 0xFF
   str r1, [r0]               // store value
   ldr r3, [r0]               // for checking

// ---------------------------
// Generic Interrupt Controller (GIC) setup - End
// ---------------------------


// ---------------------------
// Private Timer setup - Begin
// ---------------------------

    // Private Timer Load Register
   ldr r0, =PRIVATE_LOAD         // load Timer Load Register
    ldr r1, =3000000            // 3M => 1 sec(start counter value)
    str r1, [r0]               // set register

    // Private Timer Control Register
     ldr r0, =PRIVATE_CONTROL      // load control register
    mov r1, #110 << 8            // Prescalar 111 (333MHz/111 = 3MHz)
    orr r1, r1, #7                 // IRQ Enable, Auto-Reload, Timer Enable
    str r1, [r0]               // set register

// ----------------------------
// Private Timer setup - End
// ----------------------------

// ----------------------------
// UART setup - Start
// ----------------------------

   ldr      r0, =0xE0001004               // load uart mode register
   mov      r1, #0x00000020               // Normal mode, 1 stop bit, No Parity
   str      r1, [r0]                  // store value

   ldr      r0, =0xE0001000               // load uart control register
   ldr      r1, [r0]                  // load value
   bic      r1, r1, #0x0000003C            // clear TXDIS, TXEN, RXDIS, RXEN
   orr      r1, r1, #0x00000028            // TXDIS = 1 TXEN = 0 RXDIS = 1 RXEN = 0(disable TX, RX)
   str      r1, [r0]                  // store value

   ldr      r0, =0xE0001018               // load uart baud rate gen register
   mov      r1, #0x0000001B               // CD = 27
   str      r1, [r0]                  // store value
   ldr      r0, =0xE0001034               // load uart baud rate divider register
   mov      r1, #0x0000000F               // BDIV = 15
   str      r1, [r0]                  // store value (115,740 bps =  50MHz / (27*(15+1))

   ldr      r0, =0xE0001000               // load uart control register
   ldr      r1, [r0]                  // load value
   orr      r1, r1, #0x00000003            // set TXRST, RXRST(reset)
   str      r1, [r0]                  // store value

   ldr      r0, =0xE0001000               // load uart control register
   ldr      r1, [r0]                  // load value
   bic      r1, r1, #0x0000003C            // clear TXDIS, TXEN, RXDIS, RXEN
   orr      r1, r1, #0x00000014            // TXDIS = 0 TXEN = 1 RXDIS = 0 RXEN = 1(enable TX, RX)
   str      r1, [r0]                  // store value

   ldr      r0, =0xE0001000               // load uart control register
   ldr      r1, =0x00000117               // 0x00000117 to uart.Control_reg0
   str      r1, [r0]                  // store value

// ----------------------------
// UART setup - End
// ----------------------------

    // Check out the counter value to make sure the counter is decrementing
   ldr r0, =PRIVATE_COUNTER
   ldr r1, [r0]
   ldr r2, [r0]
   ldr r3, [r0]
   ldr r4, [r0]
   ldr r5, [r0]
   ldr r6, [r0]
   ldr r7, [r0]
   ldr r8, [r0]

   ldr r0, =PRIVATE_COUNTER
   ldr r1, =PRIVATE_STATUS
   ldr r2, =GICD_ISPENDR0

forever:
   ldr r5, [r0]
   ldr r6, [r1]
   ldr r7, [r2]
   b forever


TX_loop:
   stmfd sp!, {r0-r12, lr}         // push registers
loop:
   ldr r0, =0xE000102C            // uart status register
   ldr r2, [r0]               // get value in status register
   and r2, r2, #0x08            // pop out emtpy flag
   cmp r2, #0x08               // if uart is not empty
   bne loop                  // wait until it is emtpy

   ldrb r3, [r1], #1            // load 1 byte from the data in r1
   ldr r4, =0xE0001030            // uart TX_FIFO data register
   strb r3, [r4]               // store 1 byte data(send)
   cmp r3, #0x00               // if currently sent 0x00
   bne loop                  // end sending

   ldmfd sp!, {r0-r12, lr}         // pop registers
   mov pc, lr                  // return to the caller


// ----------------------------
// Interrupt Service Routines (ISRs) - Begin
// ----------------------------

csd_IRQ_ISR:

   stmfd sp!, {r0-r12, lr}         // push before running ISR

     ldr r0, =GICC_IAR            // loading address of ACK register
   ldr r4, [r0]               // Interrupt Ack(alerting GIC that ISR for interrupt # started)

   // Calling convention(r0, r1, r2, r3) <- parameter
   ldr r0, =hour               // load hour(00 for first ISR)
   ldr r1, =min               // load minute(00 for first ISR)
   ldr r2, =sec               // load second(00 for first ISR)
   ldr r3, =flag				// flag for indicating start

   bl csd_main                  // update clock(add 1 second)

   ldr r1, =hour               // load updated hour
   ldr r2, =min               // load updated minute
   ldr r3, =sec               // load updated second

   bl TX_loop                  // print hour
   ldr r1, =colon               // load ":" (error might occur)
   ldr r1, =colon2               // load ":" (error might occur)
   ldr r1, =colon3               // load ":"
   bl TX_loop                  // print(send to UART)

   mov r1, r2                  // r2(min) to r1
   bl TX_loop                  // print min
   ldr r1, =colon               // load ":" (error might occur)
   ldr r1, =colon2               // load ":" (error might occur)
   ldr r1, =colon3               // load ":"
   bl TX_loop                  // print(send to UART)

   mov r1, r3                  // r3(sec) to r1
   bl TX_loop                  // print sec
   ldr r1, =cr_return            // load "\r" (error might occur)
   ldr r1, =cr_return2            // load "\r" (error might occur)
   ldr r1, =cr_return3            // load "\r" (error might occur)
   bl TX_loop                  // print(send to UART)

    // Clear Interrupt Status bit
     ldr r0, =PRIVATE_STATUS	// interrupt status register
     mov r1, #1					// clear interrupt bit(flag)
   str r1, [r0]					// store value

    // End-of-Interrupt
     ldr r0, =GICC_EOIR            // loading address of End Of Interrupt register
   str r4, [r0]               // ISR End

   ldmfd sp!, {r0-r12, lr}         // pop registers
   subs pc, lr, #4               // return to the next instruction before Interrupt occured

// ----------------------------
// Interrupt Service Routines (ISRs) - End
// ----------------------------

.data
.align 4

irq_stack:     .space 1024
irq_stack_top:
fiq_stack:     .space 1024
fiq_stack_top:
svc_stack:     .space 1024
svc_stack_top:

colon:
   .ascii ":"
   .byte 0x00
.align 2

colon2:
   .ascii ":"
   .byte 0x00
.align 2

colon3:
   .ascii ":"
   .byte 0x00
.align 2

hour:
   .ascii "00"
   .byte 0x00
.align 2

min:
   .ascii "00"
   .byte 0x00
.align 2

sec:
   .ascii "00"
   .byte 0x00
.align 2

flag:
   .ascii "1"
   .byte 0x00
.align 2


cr_return:
   .ascii "\r"
   .byte 0x00
.align 2

cr_return2:
   .ascii "\r"
   .byte 0x00
.align 2

cr_return3:
   .ascii "\r"
   .byte 0x00
.align 2

