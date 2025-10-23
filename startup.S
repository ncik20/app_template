###########################################################################
# Sample Program for MieruEMB System v1.0            Arch Lab. TOKYO TECH #
###########################################################################
	.text
	.globl  _start
	#     .ent    _start
	_start:
	#     .set    noreorder
	#     .set    noat

	nop
	add x0, x0, x0
	add x1, x0, x0
	add x2, x0, x0
	add x3, x0, x0
	add x4, x0, x0
	add x5, x0, x0
	add x6, x0, x0
	add x7, x0, x0
	add x8, x0, x0
	add x9, x0, x0
	add x10, x0, x0
	add x11, x0, x0
	add x12, x0, x0
	add x13, x0, x0
	add x14, x0, x0
	add x15, x0, x0
	add x16, x0, x0
	add x17, x0, x0
	add x18, x0, x0
	add x19, x0, x0
	add x20, x0, x0
	add x21, x0, x0
	add x22, x0, x0
	add x23, x0, x0
	add x24, x0, x0
	add x25, x0, x0
	add x26, x0, x0
	add x27, x0, x0
	add x28, x0, x0
	add x29, x0, x0
	add x30, x0, x0
	add x31, x0, x0
	
	li sp, 0x1f00   # stack pointer 7KB
#set ps2 interrupt enable
	li x28, 0x40000201
	li x29, 0x1
	sw x29, 0(x28)
#set ps2 interrupt enable

#set PLIC Edge/Level
	li x28, 0x40000000
	sw x0, 0(x28)       #每个中断源都设置为Level类型
#set PLIC Edge/Level

#set PLIC Interrupt Priority
	li x28, 0x40000004
	li x29, 0x01111111  #每个中断源占4bit，7个中断源的优先级都设为1
	sw x29, 0(x28)
#set PLIC Interrupt Priority

#set PLIC Interrupt Enable
	li x28, 0x40000008
	li x29, 0x40        #设置中断源[6]为enable
	sw x29, 0(x28)
#set PLIC Interrupt Enable

#set PLIC Priority Threshold
	li x28, 0x4000000c
	sw x0, 0(x28)       #不屏蔽任何src
#set PLIC Priority Threshold

	j      main       # jump to the main
	nop

	.end _start
