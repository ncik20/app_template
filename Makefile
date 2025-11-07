######################################################################
# MieruEMB System V1.0  2011-10-01                Arch Lab. TOKYO TECH
######################################################################

TARGET   = init

ROOTSRC=$(wildcard *.c)
ROOTOBJ=$(patsubst %.c, %.o, $(ROOTSRC))
SUBDIR=$(shell ls -d */)
SUBSRC=$(shell find $(SUBDIR) -name '*.c')
SUBOBJ=$(SUBSRC:%.c=%.o)

$(info ROOTSRC: $(ROOTSRC))
$(info ROOTOBJ: $(ROOTOBJ))
$(info SUBDIR: $(SUBDIR))
$(info SUBSRC: $(SUBSRC))
$(info SUBOBJ: $(SUBOBJ))

OBJS = startup.o interrupt.o main.o $(SUBOBJ)
#CMDPREF = /home/share/cad/mipsel-emb/usr/bin/
CMDPREF = 

MIPSCC  = $(CMDPREF)riscv64-unknown-elf-gcc
MIPSAS  = $(CMDPREF)riscv64-unknown-elf-as
MIPSLD  = $(CMDPREF)riscv64-unknown-elf-ld
OBJDUMP = $(CMDPREF)riscv64-unknown-elf-objdump
OBJCOPY = $(CMDPREF)riscv64-unknown-elf-objcopy

MEMGEN  = ../../../../../toolchain/memgen-v0.9/memgen

CFLAGS  = -march=rv32i_zicsr -mabi=ilp32 -O0
AFLAGS  = -march=rv32i_zicsr -mabi=ilp32
LFLAGS  = -static -melf32lriscv

.SUFFIXES:
.SUFFIXES: .o .c .S
######################################################################
all:
	$(MAKE) $(TARGET)
	$(MAKE) image

$(TARGET): $(OBJS)
	$(MIPSLD) $(LFLAGS) -T stdld.script $(OBJS) -o $(TARGET)

.c.o:
#	$(MIPSCC) $(CFLAGS) -c $(@D)/$(<F) -o $(@D)/$(@F)
	$(MIPSCC) $(CFLAGS) -c $< -o $@

.S.o:
	$(MIPSAS) $(AFLAGS) $(@D)/$(<F) -o $(@D)/$(@F)

image:
	$(MEMGEN) -b $(TARGET) 16 > $(TARGET).bin
	
dump:
	$(OBJDUMP) -S $(TARGET)

copy:
	cp init.bin ../../bin

read:
	readelf -a $(TARGET)

clean:
	rm -f *.o *~ log.txt $(SUBOBJ) $(TARGET) $(TARGET).bin
######################################################################
