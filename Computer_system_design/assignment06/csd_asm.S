// ------------------------------------------
//  Author: Prof. Taeweon Suh
//          Computer Science & Engineering
//          College of Informatics, Korea Univ.
//  Date:   May 29, 2022
// ------------------------------------------

// Vector table base location should be aligned at 2^5
.align 5
// --------------------------
// Our interrupt vector table
// --------------------------
csd_vector_table:
	b .
	b .
	b .
	b csd_prefetch_abort
	b csd_data_abort
	b .
	b .
	b .

.global main
main:
	// -------------------------------------
	// Set VBAR (Vector Base Address Register) to
	// the base location of our interrupt vector table
	// -------------------------------------
	ldr     r0, =csd_vector_table
	mcr     p15, 0, r0, c12, c0, 0
	dsb
	isb

	// read SCTLR (System Control Register) to r0
	mrc	p15, 0, r0, c1, c0, 0

	// read SCR (Secure Configuration Register) to r1
	mrc p15, 0, r1, c1, c1, 0

	// read TTBCR (Translation Table Base Control Register) to r2
	mrc p15, 0, r2, c2, c0, 2

	// read TTBR0 to r3
	mrc p15, 0, r3, c2, c0, 0

	// read DACR (Domain Access Control Register) to r4
	mrc p15, 0, r4, c3, c0, 0

	/* Disable MMU, if enabled */
	mrc	p15, 0, r0, c1, c0, 0		/* read CP15 register 1 */
	bic	r0, r0, #0x1				/* clear bit 0 */
	mcr	p15, 0, r0, c1, c0, 0		/* write value back */

	/* Invalidate caches and TLBs */
	mov	r0,#0						/* r0 = 0  */
	mcr	p15, 0, r0, c8, c7, 0		/* invalidate TLBs */
	mcr	p15, 0, r0, c7, c5, 0		/* invalidate icache */
	mcr	p15, 0, r0, c7, c5, 6		/* Invalidate branch predictor array */

    /* Begin storing from -1024 to 2048*/
    mov r0, #0x400000				// from location 0x400000 (Same as PA)
	ldr r1, =-1024					// load -1024

loop:
	cmp r1, #0						// if r1 == 0
	addeq r1, r1, #1				// increment r1
	beq loop						// don't store 0 in memory
	str r1, [r0]					// store r1(-1024 ~ 2048)
	add r1, r1, #1					// increment r1 by 1(next number)
	add r0, r0, #4					// increment r0 by 4(next word)
	cmp r1, #2048					// if r1 < 2048
	ble loop						// continue storing

    /* End storing*/

	ldr	r0,=csd_MMUTable	  		/* Load MMU translation table base */
	orr	r0, r0, #0x5B				/* Outer-cacheable, WB */
	mcr	15, 0, r0, c2, c0, 0		/* TTBR0 */

	// Clear PD0 in TTBCR (Translation Table Base Control Register)
	mrc p15, 0, r2, c2, c0, 2
	bic r2, r2, #(1<<4)   			// Set PD0 to 0 = HW-based page table walk
	mcr p15, 0, r2, c2, c0, 2

	/* Enable MMU */
	// Set M-bit in SCTLR (System Control Register)
	mrc	p15, 0, r0, c1, c0, 0
	bic r0, r0, #(1<<12)  			// I-bit = I$ disable
	bic r0, r0, #(1<<2)   			// C-bit = $ disable
	orr r0, r0, #(1<<0)   			// M-bit = MMU enable
	mcr	p15, 0, r0, c1, c0, 0		/* Enable MMU */
	dsb					        	/* dsb	allow the MMU to start up */
	isb					        	/* isb	flush prefetch buffer */

	// read SCTLR (System Control Register) to r0
	mrc	p15, 0, r0, c1, c0, 0

	mov r7, #0x200000				// start from VA 0x200000
	mov r8, #0						// register to accumulate sum
	ldr r9, =0x203000				// add until VA 0x203000

add2:
	ldr r6, [r7]					// load from VA 0x200000(0x400000 in PA)
	add r8, r8, r6					// add value into r8(sum)
	add r7, r7, #4					// next word
	cmp r7, r9						// compare
	blt add2						// add until 202FFF

// ------------------------------------------------------------------------------

/* Disable MMU, if enabled */
	mrc	p15, 0, r0, c1, c0, 0		/* read CP15 register 1 */
	bic	r0, r0, #0x1				/* clear bit 0 */
	mcr	p15, 0, r0, c1, c0, 0		/* write value back */

	/* Invalidate caches and TLBs */
	mov	r0,#0						/* r0 = 0  */
	mcr	p15, 0, r0, c8, c7, 0		/* invalidate TLBs */
	mcr	p15, 0, r0, c7, c5, 0		/* invalidate icache */
	mcr	p15, 0, r0, c7, c5, 6		/* Invalidate branch predictor array */

	ldr	r0,=csd_MMUTable2	   		/* Load MMU translation table base */
	orr	r0, r0, #0x5B				/* Outer-cacheable, WB */
	mcr	15, 0, r0, c2, c0, 0		/* TTBR0 */

	// Clear PD0 in TTBCR (Translation Table Base Control Register)
	mrc p15, 0, r2, c2, c0, 2
	bic r2, r2, #(1<<4)   			// Set PD0 to 0 = HW-based page table walk
	mcr p15, 0, r2, c2, c0, 2

	/* Enable MMU */
	// Set M-bit in SCTLR (System Control Register)
	mrc	p15, 0, r0, c1, c0, 0
	bic r0, r0, #(1<<12)  			// I-bit = I$ disable
	bic r0, r0, #(1<<2)   			// C-bit = $ disable
	orr r0, r0, #(1<<0)   			// M-bit = MMU enable
	mcr	p15, 0, r0, c1, c0, 0		/* Enable MMU */
	dsb					        	/* dsb	allow the MMU to start up */
	isb					        	/* isb	flush prefetch buffer */

	mov r7, #0x200000				// start from VA 0x200000 (PA 0x400000)
	mov r8, #0						// register to accumulate sum
	ldr r9, =0x203000				// add until VA 0x203000

add3:
	ldr r6, [r7]					// load from VA 0x200000(0x400000 in PA)
	add r8, r8, r6					// add value into r8(sum)
	add r7, r7, #4					// next word
	cmp r7, r9						// compare
	blt add3						// add until 202FFF


forever:
	nop								// do nothing
	b forever						// forever loop


csd_prefetch_abort:
	mrc p15, 0, r10, c6, c0, 2  // read IFAR to r10
	mrc p15, 0, r11, c5, c0, 1  // read IFSR to r11
	b .

csd_data_abort:
	mrc p15, 0, r10, c6, c0, 0  // read DFAR to r10
	mrc p15, 0, r11, c5, c0, 0  // read DFSR to r11
	b .
