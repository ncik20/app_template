###########################################################################
# Sample Program for MieruEMB System v1.0            Arch Lab. TOKYO TECH #
###########################################################################
	.text

	li x31, 0x40000010
	lw x29, 0(x31)          # PLIC claim

    #read ps2 data
	li x28, 0x40000200
	lw x29, 0(x28)

    lb x28, kb_wptr         # load offset
    addi x30, x28, 0x10     # kb_buffer base addr(0x10) + offset
    sb x29, 0(x30)          # store ps2 data to kb_buffer 
    addi x28, x28, 0x1      # offset + 1
    sw x28, kb_wptr, x30    # store offset to kb_wptr

    sw x0, 0(x31)           # PLIC done

    mret

#    csrr x31, 0x341         # get mepc

#    csrsi 0x300, 0b1000     # set mie enable

#	jr x31
#	nop

	.data

kb_wptr:
    .byte 0x0
