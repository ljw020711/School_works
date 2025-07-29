// ------------------------------------------
//  Author: Prof. Taeweon Suh
//          Computer Science & Engineering
//          College of Informatics, Korea Univ.
//  Date:   May 06, 2020
// ------------------------------------------

#include "csd_zynq_peripherals.h"

.align 5

csd_vector_table:
   b .
   b .
   b .
   b .
   b .
   b .
   b csd_IRQ_ISR			// ISR for timer interrupt
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

   cps #0x13                // supervisor mode
// ---------------------------
// Generic Interrupt Controller (GIC) setup - Begin
// ---------------------------

   // CPU Interface ID Register
   ldr r0, =GICC_IIDR			// checking
   ldr r3, [r0]					// checking

   // CPU Controller Type Register
   ldr r0, =GICD_TYPER			// checking
   ldr r3, [r0]					// checking

   // CPU Binary Pointer Register
   ldr r0, =GICC_BPR			// checking
   ldr r3, [r0]					// checking

   // Distributor Control Register
   ldr r0, =GICD_CTLR
   ldr r1, [r0]
   mov r2, #1                   // Enable
   orr r1, r1, r2				// Enable the certain bit
   str r1, [r0]					// store
   ldr r3, [r0]					// checking

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
   orr r1, r1, r2				// Enable the certain bit
   str r1, [r0]					// store
   ldr r3, [r0]					// for checking

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
    ldr r1, =3000               // 3000 => 0.001 sec(start counter value)
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

// stack & tcb setup for each user program
   ldr r0, =task1_tcb					// get address of tcb for task1
   ldr r1, =task1_stack_top				// get the stack top address(for sp)
   add r0, r0, #52                     // r13(sp)
   str r1, [r0]							// store task1_stack_top
   add r0, r0, #4                     // r14(lr)
   ldr r1, =forever						// get forever loop(not used)
   str r1, [r0]							// store the return location
   add r0, r0, #4                     // r15(pc)
   ldr r1, =task1_c						// get address of task1_c.c
   add r1, r1, #4                     // subs pc, lr, #4 (so it should point 1 byte above)
   str r1, [r0]							// store it in pc(resume location, initially start instruction)
   add r0, r0, #4						// spsr
   ldr r1, =0x60000050					// nZCv, f = 1, user mode
   str r1, [r0]							// store spsr

   ldr r0, =task2_tcb					// get address of tcb for task2
   ldr r1, =task2_stack_top				// get the stack top address(for sp)
   add r0, r0, #52                     // r13(sp)
   str r1, [r0]							// store task2_stack_top
   add r0, r0, #4                     // r14(lr)
   ldr r1, =forever						// get forever loop(not used)
   str r1, [r0]							// store the return location
   add r0, r0, #4                     // r15(pc)
   ldr r1, =task2_c						// get address of task2_c.c
   add r1, r1, #4                     // subs pc, lr, #4 (so it should point 1 byte above)
   str r1, [r0]							// store it in pc(resume location, initially start instruction)
   add r0, r0, #4						// spsr
   ldr r1, =0x60000050					// nZCv, f = 1, user mode
   str r1, [r0]							// store spsr

   ldr r0, =task3_tcb					// get address of tcb for task2
   ldr r1, =task3_stack_top				// get the stack top address(for sp)
   add r0, r0, #52                     // r13(sp)
   str r1, [r0]							// store task2_stack_top
   add r0, r0, #4                     // r14(lr)
   ldr r1, =forever						// get forever loop(not used)
   str r1, [r0]							// store the return location
   add r0, r0, #4                     // r15(pc)
   ldr r1, =task3_c						// get address of task2_c.c
   add r1, r1, #4                     // subs pc, lr, #4 (so it should point 1 byte above)
   str r1, [r0]							// store it in pc(resume location, initially start instruction)
   add r0, r0, #4						// spsr
   ldr r1, =0x60000050					// nZCv, f = 1, user mode
   str r1, [r0]							// store spsr

   ldr r0, =task1_tcb					// r0 = address of task2_tcb
   add r0, r0, #4
   ldm r0, {r1-r14}^					// load register values from tcb(r1-lr)
   stmfd sp!, {r1}						// store r1
   mov r1, r0							// r1 pointing r0 in tcb
   ldm r1, {r0}							// load r0 from tcb
   ldmfd sp!, {r1}						// load r1

   cps #0x10							// set user mode
   b task1_c							// run task1

forever:
   nop									// do
   b forever							// nothing

// ----------------------------
// Interrupt Service Routines (ISRs) - Begin
// ----------------------------

csd_IRQ_ISR:
   stmfd sp!, {r0-r12, lr}         // push before running ISR

     ldr r0, =GICC_IAR            // loading address of ACK register
   ldr r4, [r0]               // Interrupt Ack(alerting GIC that ISR for interrupt # started)
   ldr r0, =ack_temp			// storing current ack number to ack_temp (for EOI)
   str r4, [r0]					// store

    // Clear Interrupt Status bit
     ldr r0, =PRIVATE_STATUS	// private timer status register
     mov r1, #1					// 1 to r1
   str r1, [r0]					// clear interrupt flag

   ldr r2, =current_task		// get current_task number
   ldrb r4, [r2]				// load
   mov r3, #0x31				// 1
   cmp r4, r3					// compare
   beq switch_t1_t2				// swtich from task1 to task2
   mov r3, #0x32				// 2
   cmp r4, r3					// compare
   beq switch_t2_t3				// swtich from task2 to task3
   mov r3, #0x33				// 3
   cmp r4, r3					// compare
   beq switch_t3_t1				// swtich from task3 to task1

switch_t1_t2:
   add sp, sp, #4				// sp = sp + 4, to load r1-r12, lr
   ldmfd sp!, {r1-r12, lr}		// load status from user mode
   ldr r0, =task1_tcb			// load task1_tcb
   add r0, r0, #4				// r0 = r0 + 4, to store from r1
   stm r0, {r1-r14}^			// store in to tcb (user mode registers)
   add r0, r0, #56				// r0 pointing pc(return location, to resume)
   stm r0!, {lr}				// store new resume address
   mrs r2, spsr					// get spsr_irg(cpsr of user mode)
   stm r0!, {r2}				// store it to spsr in tcb
   sub r1, r0, #68				// r1 pointing r0 in tcb
   sub sp, sp, #56				// sp pointing r0 in stack
   ldmfd sp!, {r0}				// load r0 to stack
   stm r1, {r0}					// store r0 to tcb
   add sp, sp, #52				// stack pointer to previous place

   ldr r0, =task2_tcb			// r0 = address of task2_tcb
   add r0, r0, #4				// r0 pointing r1 in tcb
   add r0, r0, #56				// r0 pointing pc in tcb
   ldr r1, [r0]               	// pc(resume)
   mov lr, r1                   // lr_irq(return from exception) to resume address
   add r0, r0, #4               // spsr
   ldr r1, [r0]					// store spsr to r1
   msr spsr, r1               	// spsr_irq <- spsr from last execution(back to cpsr_user by hardware)


   ldr r0, =ack_temp			// get ack number
   ldr r4, [r0]					// load

    // End-of-Interrupt
   ldr r0, =GICC_EOIR            // loading address of End Of Interrupt register
   str r4, [r0]               // ISR End

   ldr r0, =current_task		// get current_task number
   ldrb r1, [r0]				// load
   add r1, r1, #1				// add 1 (pointing next task)
   cmp r1, #0x34				// if the ascii is over 0x33(3)
   moveq r1, #0x31				// change it into 0x31(1)
   strb r1, [r0]				// store

   ldr r0, =task2_tcb			// r0 = address of task2_tcb
   add r0, r0, #4				// r0 pointing r1 in tcb
   ldm r0, {r1-r14}^			// load register values from tcb(r1-lr)
   stmfd sp!, {r1}				// store r1
   mov r1, r0					// r1 pointing r0 in tcb
   ldm r1, {r0}					// load r0 from tcb
   ldmfd sp!, {r1}				// load r1

   b done						// branch to done

switch_t2_t3:
   add sp, sp, #4				// sp = sp + 4, to load r1-r12, lr
   ldmfd sp!, {r1-r12, lr}		// load status from user mode
   ldr r0, =task2_tcb			// load task1_tcb
   add r0, r0, #4				// r0 = r0 + 4, to store from r1
   stm r0, {r1-r14}^			// store in to tcb (user mode registers)
   add r0, r0, #56				// r0 pointing pc(return location, to resume)
   stm r0!, {lr}				// store new resume address
   mrs r2, spsr					// get spsr_irg(cpsr of user mode)
   stm r0!, {r2}				// store it to spsr in tcb
   sub r1, r0, #68				// r1 pointing r0 in tcb
   sub sp, sp, #56				// sp pointing r0 in stack
   ldmfd sp!, {r0}				// load r0 to stack
   stm r1, {r0}					// store r0 to tcb
   add sp, sp, #52				// stack pointer to previous place

   ldr r0, =task3_tcb			// r0 = address of task2_tcb
   add r0, r0, #4				// r0 pointing r1 in tcb
   add r0, r0, #56				// r0 pointing pc in tcb
   ldr r1, [r0]               	// pc(resume)
   mov lr, r1                   // lr_irq(return from exception) to resume address
   add r0, r0, #4               // spsr
   ldr r1, [r0]					// store spsr to r1
   msr spsr, r1               	// spsr_irq <- spsr from last execution(back to cpsr_user by hardware)


   ldr r0, =ack_temp			// get ack number
   ldr r4, [r0]					// load

    // End-of-Interrupt
   ldr r0, =GICC_EOIR            // loading address of End Of Interrupt register
   str r4, [r0]               // ISR End

   ldr r0, =current_task		// get current_task number
   ldrb r1, [r0]				// load
   add r1, r1, #1				// add 1 (pointing next task)
   cmp r1, #0x34				// if the ascii is over 0x33(3)
   moveq r1, #0x31				// change it into 0x31(1)
   strb r1, [r0]				// store

   ldr r0, =task3_tcb			// r0 = address of task2_tcb
   add r0, r0, #4				// r0 pointing r1 in tcb
   ldm r0, {r1-r14}^			// load register values from tcb(r1-lr)
   stmfd sp!, {r1}				// store r1
   mov r1, r0					// r1 pointing r0 in tcb
   ldm r1, {r0}					// load r0 from tcb
   ldmfd sp!, {r1}				// load r1

   b done						// branch to done

switch_t3_t1:
   add sp, sp, #4				// sp = sp + 4, to load r1-r12, lr
   ldmfd sp!, {r1-r12, lr}		// load status from user mode
   ldr r0, =task3_tcb			// load task1_tcb
   add r0, r0, #4				// r0 = r0 + 4, to store from r1
   stm r0, {r1-r14}^			// store in to tcb (user mode registers)
   add r0, r0, #56				// r0 pointing pc(return location, to resume)
   stm r0!, {lr}				// store new resume address
   mrs r2, spsr					// get spsr_irg(cpsr of user mode)
   stm r0!, {r2}				// store it to spsr in tcb
   sub r1, r0, #68				// r1 pointing r0 in tcb
   sub sp, sp, #56				// sp pointing r0 in stack
   ldmfd sp!, {r0}				// load r0 to stack
   stm r1, {r0}					// store r0 to tcb
   add sp, sp, #52				// stack pointer to previous place

   ldr r0, =task1_tcb			// r0 = address of task2_tcb
   add r0, r0, #4				// r0 pointing r1 in tcb
   add r0, r0, #56				// r0 pointing pc in tcb
   ldr r1, [r0]               	// pc(resume)
   mov lr, r1                   // lr_irq(return from exception) to resume address
   add r0, r0, #4               // spsr
   ldr r1, [r0]					// store spsr to r1
   msr spsr, r1               	// spsr_irq <- spsr from last execution(back to cpsr_user by hardware)


   ldr r0, =ack_temp			// get ack number
   ldr r4, [r0]					// load

    // End-of-Interrupt
   ldr r0, =GICC_EOIR            // loading address of End Of Interrupt register
   str r4, [r0]               // ISR End

   ldr r0, =current_task		// get current_task number
   ldrb r1, [r0]				// load
   add r1, r1, #1				// add 1 (pointing next task)
   cmp r1, #0x34				// if the ascii is over 0x33(3)
   moveq r1, #0x31				// change it into 0x31(1)
   strb r1, [r0]				// store

   ldr r0, =task1_tcb			// r0 = address of task2_tcb
   add r0, r0, #4				// r0 pointing r1 in tcb
   ldm r0, {r1-r14}^			// load register values from tcb(r1-lr)
   stmfd sp!, {r1}				// store r1
   mov r1, r0					// r1 pointing r0 in tcb
   ldm r1, {r0}					// load r0 from tcb
   ldmfd sp!, {r1}				// load r1

   b done						// branch to done

done:

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

task1_stack:   .space 1024
task1_stack_top:
task2_stack:   .space 1024
task2_stack_top:
task3_stack:   .space 1024
task3_stack_top:

task1_tcb: .word 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0		// tcb from r0-r15, spsr
task2_tcb: .word 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
task3_tcb: .word 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

current_task:
   .ascii "1"
   .byte 0x00
   .align 2

ack_temp: .word 0
