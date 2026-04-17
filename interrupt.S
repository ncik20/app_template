###########################################################################
# Sample Program for MieruEMB System v1.0            Arch Lab. TOKYO TECH #
###########################################################################
.global freertos_risc_v_trap_handler

.extern kb_wptr
.extern PLIC_IDRegister
.extern PS2_DATARegister
.extern pullMachineTimerCompareRegister
.extern pullNextTime
.extern uxTimerIncrementsForOneTick

   .macro portcontextSAVE_CONTEXT_INTERNAL
addi sp, sp, -120
SW x1,  2  * 4( sp )
SW x5,  3  * 4( sp )
SW x6,  4  * 4( sp )
SW x7,  5  * 4( sp )
SW x8,  6  * 4( sp )
SW x9,  7  * 4( sp )
SW x10, 8  * 4( sp )
SW x11, 9  * 4( sp )
SW x12, 10 * 4( sp )
SW x13, 11 * 4( sp )
SW x14, 12 * 4( sp )
SW x15, 13 * 4( sp )
SW x16, 14 * 4( sp )
SW x17, 15 * 4( sp )
SW x18, 16 * 4( sp )
SW x19, 17 * 4( sp )
SW x20, 18 * 4( sp )
SW x21, 19 * 4( sp )
SW x22, 20 * 4( sp )
SW x23, 21 * 4( sp )
SW x24, 22 * 4( sp )
SW x25, 23 * 4( sp )
SW x26, 24 * 4( sp )
SW x27, 25 * 4( sp )
SW x28, 26 * 4( sp )
SW x29, 27 * 4( sp )
SW x30, 28 * 4( sp )
SW x31, 29 * 4( sp )

   .endm
/*-----------------------------------------------------------*/

   .macro portcontextSAVE_INTERRUPT_CONTEXT
portcontextSAVE_CONTEXT_INTERNAL
   .endm
/*-----------------------------------------------------------*/

   .macro portcontextRESTORE_CONTEXT
LW x1,  2  * 4( sp )
LW x5,  3  * 4( sp )
LW x6,  4  * 4( sp )
LW x7,  5  * 4( sp )
LW x8,  6  * 4( sp )
LW x9,  7  * 4( sp )
LW x10, 8  * 4( sp )
LW x11, 9  * 4( sp )
LW x12, 10 * 4( sp )
LW x13, 11 * 4( sp )
LW x14, 12 * 4( sp )
LW x15, 13 * 4( sp )
LW x16, 14 * 4( sp )
LW x17, 15 * 4( sp )
LW x18, 16 * 4( sp )
LW x19, 17 * 4( sp )
LW x20, 18 * 4( sp )
LW x21, 19 * 4( sp )
LW x22, 20 * 4( sp )
LW x23, 21 * 4( sp )
LW x24, 22 * 4( sp )
LW x25, 23 * 4( sp )
LW x26, 24 * 4( sp )
LW x27, 25 * 4( sp )
LW x28, 26 * 4( sp )
LW x29, 27 * 4( sp )
LW x30, 28 * 4( sp )
LW x31, 29 * 4( sp )
addi sp, sp, 120

mret
   .endm

.macro portUPDATE_MTIMER_COMPARE_REGISTER
    lw a0, pullMachineTimerCompareRegister  /* Load address of compare register into a0. */
    lw a1, pullNextTime                     /* Load the address of ullNextTime into a1. */

    /* Update the 64-bit mtimer compare match value in two 32-bit writes. */
    li a4, -1
    lw a2, 0(a1)                /* Load the low word of ullNextTime into a2. */
    lw a3, 4(a1)                /* Load the high word of ullNextTime into a3. */
    sw a4, 0(a0)                /* Low word no smaller than old value to start with - will be overwritten below. */
    sw a3, 4(a0)                /* Store high word of ullNextTime into compare register.  No smaller than new value. */
    sw a2, 0(a0)                /* Store low word of ullNextTime into compare register. */
    lw t0, uxTimerIncrementsForOneTick  /* Load the value of ullTimerIncrementForOneTick into t0 (could this be optimized by storing in an array next to pullNextTime?). */
    add a4, t0, a2              /* Add the low word of ullNextTime to the timer increments for one tick (assumes timer increment for one tick fits in 32-bits). */
    sltu t1, a4, a2             /* See if the sum of low words overflowed (what about the zero case?). */
    add t2, a3, t1              /* Add overflow to high word of ullNextTime. */
    sw a4, 0(a1)                /* Store new low word of ullNextTime. */
    sw t2, 4(a1)                /* Store new high word of ullNextTime. */
    .endm

###########################################################################
# interrupt                                                               #
###########################################################################
.section .text.freertos_risc_v_trap_handler
.align 8
freertos_risc_v_trap_handler:
    portcontextSAVE_INTERRUPT_CONTEXT

    li   t0, 0x800
    csrr t1, mip
    and  t0, t1, t0         # test MEIP
    bnez t0, external_interrupt_handler

time_interrupt_handler:
    portUPDATE_MTIMER_COMPARE_REGISTER
    portcontextRESTORE_CONTEXT

external_interrupt_handler:
    #li x31, 0x40010010
    lw x31, PLIC_IDRegister
    lw x29, 0(x31)          # PLIC claim 

    li x31, 0x7
    beq x29, x31, keyboard_irq
    j do_nothing

keyboard_irq:
    #read ps2 data
    #li x28, 0x40020000
    lw x28, PS2_DATARegister
    lw x29, 0(x28)

    lbu x28, kb_wptr        # load offset
    addi x30, x28, 0x300    # kb_buffer base addr(0x300~0x3ff,256byte) + offset
    sb x29, 0(x30)          # store ps2 data to kb_buffer 
    addi x28, x28, 0x1      # offset + 1
    sb x28, kb_wptr, x30    # store offset to kb_wptr

do_nothing:
    #li x31, 0x40010010
    lw x31, PLIC_IDRegister
    sw x0, 0(x31)           # PLIC done

    portcontextRESTORE_CONTEXT

