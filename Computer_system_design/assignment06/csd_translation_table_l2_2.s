// ------------------------------------------
//  Author: Prof. Taeweon Suh
//          Computer Science & Engineering
//          College of Informatics, Korea Univ.
//  Date:   June 01, 2022
//
//  It is based on Boot code in Xilinx SDK
// ------------------------------------------

.globl  csd_MMUTable_l2_2
.section .csd_mmu_l2_tbl2,"a"

csd_MMUTable_l2_2:
	/* A 32-bit is required for each PTE (Page Table Entry).
	 * Each PTE covers a 1MB section.
	 * There are 4096 PTEs, so 16KB is required for the page table.
	 *
	 *  First 6 PTEs with the following translations
	 *     1st 4KB: 0x0020_0000 (VA) -> 0x0040_0000 (PA)
	 *     2nd 1MB: 0x0020_1000 (VA) -> 0x0040_1000 (PA)
	 *     3rd 1MB: 0x0020_2000 (VA) -> 0x0040_2000 (PA)
	 *     4th 1MB: 0x0020_3000 (VA) -> 0x0040_3000 (PA)
	 *     5th 1MB: 0x0020_4000 (VA) -> 0x0040_4000 (PA)
	 *     6th 1MB: 0x0020_5000 (VA) -> 0x0040_5000 (PA)
	 */
.set SECT, 0x400000
.word	SECT + 0x576			/* S=b1 TEX=b101 AP=b11, C=b0, B=b1, XN=b10 */
.set	SECT, SECT + 0x2000		// PA = 0x402000
.word	SECT + 0x576			/* S=b1 TEX=b101 AP=b11, C=b0, B=b1, XN=b10 */
.set	SECT, SECT - 0x2000		// PA = 0x400000
.word	SECT + 0x576			/* S=b1 TEX=b101 AP=b11, C=b0, B=b1, XN=b10 */
.set	SECT, SECT + 0x1000		// PA = 0x401000
.word	SECT + 0x576			/* S=b1 TEX=b101 AP=b11, C=b0, B=b1, XN=b10 */
.set	SECT, SECT + 0x1000		// PA = 0x402000
.word	SECT + 0x576			/* S=b1 TEX=b101 AP=b11, C=b0, B=b1, XN=b10 */
.set	SECT, SECT + 0x1000		// PA = 0x403000
.word	SECT + 0x576			/* S=b1 TEX=b101 AP=b11, C=b0, B=b1, XN=b10 */

.rept (0x100 - 6)				// fill rest of 250 entries
.word	SECT + 0x576			/* S=b1 TEX=b101 AP=b11, C=b0, B=b1, XN=b10 */
.endr

.end
