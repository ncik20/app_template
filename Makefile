######################################################################
# MieruEMB System V1.0  2011-10-01                Arch Lab. TOKYO TECH
######################################################################

TARGET = init

#ROOTSRC=$(wildcard *.c)
#ROOTOBJ=$(patsubst %.c, %.o, $(ROOTSRC))
SUBDIR=$(shell ls -d */)
SUBCSRC=$(shell find $(SUBDIR) -name '*.c')
SUBASSRC=$(shell find $(SUBDIR) -name '*.S')
SUBCOBJ=$(SUBCSRC:%.c=%.o)
SUBASOBJ=$(SUBASSRC:%.S=%.o)

#$(info ROOTSRC: $(ROOTSRC))
#$(info ROOTOBJ: $(ROOTOBJ))
$(info SUBDIR: $(SUBDIR))
$(info SUBCSRC: $(SUBCSRC))
$(info SUBASSRC: $(SUBASSRC))
$(info SUBCOBJ: $(SUBCOBJ))
$(info SUBASOBJ: $(SUBASOBJ))

#OBJS = startup.o interrupt.o main.o $(SUBCOBJ) $(SUBASOBJ)
OBJS = startup.o main.o $(SUBCOBJ) $(SUBASOBJ)

CMDPREF =

MIPSCC  = $(CMDPREF)riscv64-unknown-elf-gcc
MIPSAS  = $(CMDPREF)riscv64-unknown-elf-as
#MIPSLD  = $(CMDPREF)riscv64-unknown-elf-ld
MIPSLD  = $(CMDPREF)riscv64-unknown-elf-gcc
OBJDUMP = $(CMDPREF)riscv64-unknown-elf-objdump
OBJCOPY = $(CMDPREF)riscv64-unknown-elf-objcopy

MEMGEN  = ../../../../../toolchain/memgen-v0.9/memgen

INCLUDE_DIRS = -I. \
               -Idrivers/inc \
			   -IHAL/inc \
			   -IHAL/inc/sys \
			   -IfreeRTOS/include

CFLAGS  = -march=rv32i_zicsr -mabi=ilp32 -O0 --specs=picolibc.specs
CFLAGS  += $(INCLUDE_DIRS)
CFLAGS  += -ffunction-sections -fdata-sections
AFLAGS  = -march=rv32i_zicsr -mabi=ilp32
#LFLAGS  = -static -melf32lriscv
LFLAGS  = -march=rv32i_zicsr -mabi=ilp32 -static -nostartfiles -nodefaultlibs -L/usr/lib/picolibc/riscv64-unknown-elf/lib/rv32i/ilp32
LFLAGS  += -Wl,--gc-sections
.SUFFIXES:
.SUFFIXES: .o .c .S
######################################################################
#Makefile STEP1
#当输入 make 时，它会去找第一个目标（通常是 all），而 all 往往指向 $(TARGET)
all:
	$(MAKE) $(TARGET)
	$(MAKE) image

#Makefile STEP2
#$(TARGET)依赖$(OBJS)，如下，都为.o文件
#OBJS = startup.o interrupt.o main.o $(SUBCOBJ) $(SUBASOBJ)
$(TARGET): $(OBJS)
	$(MIPSLD) $(LFLAGS) -T stdld.script $(OBJS) -o $(TARGET) -lc

#Makefile STEP3
#根据以下规则（.c.o、.S.o）以及STEP2决定需要哪些.o文件，选择编译相应的.c和.S文件
.c.o:
	$(MIPSCC) $(CFLAGS) -c $< -o $@

.S.o:
	$(MIPSCC) $(AFLAGS) -c $< -o $@

image:
	$(OBJCOPY) -O binary $(TARGET) $(TARGET).bin
	
dump:
	$(OBJDUMP) -S $(TARGET)

copy:
	cp init.bin ../../bin

read:
	readelf -a $(TARGET)

clean:
	rm -f *.o *~ log.txt $(SUBCOBJ) $(SUBASOBJ) $(TARGET) $(TARGET).bin hexdump_output a.out init.mif
######################################################################
