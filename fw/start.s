#include "AT91SAM7S64.H"
#include "arm.h"
#include "config.h"

			.align		4
			.arm
/* Exception vectors
 *******************/
		.section	.vectors, "a"
resetVector:	ldr	pc, Reset_Addr
undefVector:	ldr	pc, Undef_Addr
swiVector:	ldr	pc, SWI_Addr
pabtVector:	ldr	pc, Prefetch_Addr
dabtVector:	ldr	pc, DABT_Addr
rsrvVector:	b	rsrvVector
irqVector:	ldr	pc, IRQ_Addr
fiqVector:	ldr	pc, FIQ_Addr

/* Jump-Table */
Reset_Addr:	.word resetHandler
Undef_Addr:	.word undefHandler
SWI_Addr:	.word swiHandler
Prefetch_Addr:.word prefetchHandler
DABT_Addr:	.word dabtHandler
Reserv_Addr:	.word 0
IRQ_Addr:	.word irqHandler
FIQ_Addr:	.word fiqHandler


undefHandler:
		b undefHandler
prefetchHandler:
		b prefetchHandler
dabtHandler:
		b dabtHandler

//------------------------------------------------------------------------------
/// Handles incoming interrupt requests by branching to the corresponding
/// handler, as defined in the AIC. Supports interrupt nesting.
//------------------------------------------------------------------------------
irqHandler:
		STMDB	R13!, {R0-R3,R12,R14}
		BL 		irqc
		LDMIA	R13!, {R0-R3,R12,R14}
		SUBS	PC, R14, #4

fiqHandler:
		STMDB	R13!, {R0-R3,R14}
		BL 		fiqc
		LDMIA	R13!, {R0-R3,R14}
		SUBS	PC, R14, #4



swiHandler:
		stmfd	sp!, {r12, lr}		// Store R12, LR
		MRS 	R12, SPSR		//Get SPSR
		STMFD	SP!, {R8, R12}	//Store R8, SPSR
		TST		R12, #T_Bit 			//Check Thumb Bit
		LDRNEH	R12, [LR,#-2]		//Thumb: Load Halfword
		BICNE	R12, R12, #0xFF00	//Extract SWI Number
		LDREQ	R12, [LR,#-4]		//ARM:	Load Word
		BICEQ	R12, R12, #0xFF000000  //Extract SWI Number

		CMP 	R12, #SWI_Count
		BHS 	SWI_Dead		//Overflow
		ADR 	R8, SWI_Table
		LDR		R12, [R8,R12,LSL #2]	//Load SWI Function Address
		MOV 	LR, PC			// Return Address
		BX		R12 				// Call SWI Function

		LDMFD	SP!, {R8, R12}	// Load R8, SPSR
		MSR 	SPSR_cxsf, R12	// Set SPSR
		LDMFD	SP!, {R12, PC}^ 	// Restore R12 and Return

SWI_Dead:
		B		SWI_Dead	//None Existing SWI

/* Jump-Table */
SWI_Table:
		.word 0	// 0
		.word usbisr_write_ep		// 1
		.word usbisr_reset_ep	// 2
		.word usbisr_flush_ep		// 3
SWI_End:
		.set SWI_Count, ((SWI_End-SWI_Table)/4)



//------------------------------------------------------------------------------
/// Initializes the chip and branches to the main() function.
//------------------------------------------------------------------------------
		.section	.text
		.global 	entry

entry:
resetHandler:

/* Dummy access to the .vectors section so it does not get optimized */
		ldr 	r0, =resetVector

/* Set pc to actual code location (i.e. not in remap zone) */
		ldr 	pc, =1f

/* Perform low-level initialization of the chip using LowLevelInit() */
1:
		ldr		r4, =SVC_Stack
		add		r4, r4, #SVC_Stack_Size
		mov 	sp, r4
		ldr		r0, =init
		mov 	lr, pc
		bx		r0

/* Initialize the relocate segment */

		ldr		r0, =_efixed
		ldr		r1, =_srelocate
		ldr		r2, =_erelocate
1:
		cmp 	r1, r2
		ldrcc		r3, [r0], #4
		strcc		r3, [r1], #4
		bcc		1b

/* Clear the zero segment */
		ldr		r0, =_szero
		ldr		r1, =_ezero
		mov		r2, #0
1:
		cmp 	r0, r1
		strcc		r2, [r0], #4
		bcc		1b

/* Setup stacks
 **************/
/* IRQ mode */
		msr 	CPSR_c, #Mode_IRQ | I_Bit | F_Bit
		ldr		r4, =IRQ_Stack
		add		r4, r4, #IRQ_Stack_Size
		mov 	sp, r4

/* FIQ mode */
		msr 	CPSR_c, #Mode_FIQ | I_Bit | F_Bit
		ldr		r4, =FIQ_Stack
		add		r4, r4, #FIQ_Stack_Size
		mov 	sp, r4
		
/* SVC mode */
		msr 	CPSR_c, #Mode_SVC | I_Bit | F_Bit
		ldr		r4, =SVC_Stack
		add		r4, r4, #SVC_Stack_Size
		mov 	sp, r4

/* USR mode(interrupts enabled) */
		msr 	CPSR_c, #Mode_USR
		ldr		r4, =USR_Stack
		add		r4, r4, #USR_Stack_Size
		mov 	sp, r4

/* Branch to main()
 ******************/
		ldr 	r0, =main
		mov 	lr, pc
		bx		r0

/* Loop indefinitely when program is finished */
exit:
		b		exit

