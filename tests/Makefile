# Cross toolchain variables
# If these are not in your path, you can make them absolute.
XT_PRG_PREFIX = riscv32-unknown-linux-gnu-
CC = $(XT_PRG_PREFIX)gcc
LD = $(XT_PRG_PREFIX)ld
OBJCOPY = $(XT_PRG_PREFIX)objcopy
OBJDUMP = $(XT_PRG_PREFIX)objdump

# uRISCV-related paths
URISCV_DIR_PREFIX = ..

URISCV_DATA_DIR = $(URISCV_DIR_PREFIX)/src/support/ldscripts
URISCV_LIB_DIR = $(URISCV_DIR_PREFIX)/src/support/liburiscv
URISCV_CRT_DIR = $(URISCV_DIR_PREFIX)/src/support/crt
URISCV_INCLUDE_DIR = $(URISCV_DIR_PREFIX)/src/include

# Compiler options
CFLAGS_LANG = -ffreestanding -ansi
CFLAGS = $(CFLAGS_LANG) -I$(URISCV_INCLUDE_DIR) -Wall -O0 -march=rv32id -mabi=ilp32d
OBJCOPYFLAGS = -O binary
OBJDUMPFLAGS = -d

# Linker options
LDFLAGS = -G 0 -nostdlib -T $(URISCV_DATA_DIR)/uriscvcore.ldscript 

# Add the location of crt*.S to the search path
VPATH = $(URISCV_CRT_DIR)  $(URISCV_LIB_DIR) 

.PHONY : all clean

all : kernel.core.uriscv

kernel.core.uriscv : kernel
	$(OBJCOPY) $(OBJCOPYFLAGS) $< $@
	# umps3-elf2umps -k $<

kernel : test.o crtso.o liburiscv.o
	$(LD) -o $@ $^ $(LDFLAGS)

dump : all
	$(OBJDUMP) $(OBJDUMPFLAGS) kernel

entry : all
	ENTRY="$(readelf -h kernel | grep Entry | awk '{print $4}')"; echo $$ENTRY

clean :
	-rm -f *.o kernel kernel.*.uriscv

# dont know why but i have to force it
test.o:
	$(CC) $(CFLAGS) -c -o test.o test.c

# Pattern rule for assembly modules
%.o : %.S
	$(CC) $(CFLAGS) -c -o $@ $<

