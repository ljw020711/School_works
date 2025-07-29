// ------------------------------------------
//  Author: Prof. Taeweon Suh
//          Computer Science & Engineering
//          College of Informatics, Korea Univ.
//  Date:   June 01, 2022
//
//  It is based on Boot code in Xilinx SDK
// ------------------------------------------

.globl  csd_MMUTable2
.section .csd_mmu_l1_tbl2,"a"

csd_MMUTable2:
	/* A 32-bit is required for each PTE (Page Table Entry).
	 * Each PTE covers a 1MB section.
	 * There are 4096 PTEs, so 16KB is required for the page table.
	 *
	 *  First 6 PTEs with the following translations
	 *     1st 1MB: 0x0000_0000 (VA) -> 0x0000_0000 (PA)
	 *     2nd 1MB: 0x0010_0000 (VA) -> 0x0020_0000 (PA)
	 *     3rd 1MB: 3rd 1MB: L1 discriptor(Page table) -> L2 page table base address[30:10]
	 *     4th 1MB: 0x0030_0000 (VA) -> 0x0020_0000 (PA)
	 *     5th 1MB: 0x0040_0000 (VA) -> 0x0010_0000 (PA)
	 *     6th 1MB: 0x0050_0000 (VA) -> 0x0050_0000 (PA)
	 */
.set SECT, 0
.word	SECT + 0x15de6				/* S=b1 TEX=b101 AP=b11, Domain=b1111, C=b0, B=b1 */
.set	SECT, SECT + 0x200000
.word	SECT + 0x15de6				/* S=b1 TEX=b101 AP=b11, Domain=b1111, C=b0, B=b1 */
.set	SECT, SECT + 0x200000
.extern csd_MMUTable_l2_2			// get address of csd_MMUTable_l2_2
.word	csd_MMUTable_l2_2 + 0x001	// L2 table base address + last 2bit = b01
.set	SECT, SECT - 0x200000
.word	SECT + 0x15de6				/* S=b1 TEX=b101 AP=b11, Domain=b1111, C=b0, B=b1 */
.set	SECT, SECT - 0x100000
.word	SECT + 0x15de6				/* S=b1 TEX=b101 AP=b11, Domain=b1111, C=b0, B=b1 */
.set	SECT, SECT + 0x400000
.word	SECT + 0x15de6				/* S=b1 TEX=b101 AP=b11, Domain=b1111, C=b0, B=b1 */

.rept (0x200 - 6)					// fill rest 4090 entries
.word	SECT + 0x15de6				/* S=b1 TEX=b101 AP=b11, Domain=b1111, C=b0, B=b1 */
.endr

.end
