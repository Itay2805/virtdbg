########################################################################################################################
# Build config
########################################################################################################################

#
# The cross compiler to use
#
TOOLCHAIN ?= toolchain/bin/x86_64-linux-

########################################################################################################################
# Build constants
########################################################################################################################

CC 		        	:= $(TOOLCHAIN)gcc
OBJCOPY 			:= $(TOOLCHAIN)objcopy
LD	        		:= $(TOOLCHAIN)gcc

CFLAGS 		:= -Wall -Werror -Wno-unused-label
CFLAGS 		+= -mno-sse -mno-sse2 -mno-mmx -mno-80387 -m64
CFLAGS 		+= -mno-red-zone -fno-builtin -march=nehalem
CFLAGS 		+= -ffreestanding -fno-asynchronous-unwind-tables
CFLAGS 		+= -Os -flto -ffat-lto-objects -g3
CFLAGS 		+= -mcmodel=kernel
CFLAGS 		+= -Ivirtdbg -Wl,--omagic -Tvirtdbg/linker.ld

CFLAGS 		+= -nostdlib -nodefaultlibs -nostartfiles
CFLAGS 		+= -z max-page-size=0x1000

SRCS		:= $(shell find virtdbg -name '*.c')
SRCS		+= $(shell find virtdbg -name '*.asm')

OUT_DIR 	:= out

BIN_DIR := $(OUT_DIR)/bin
BUILD_DIR := $(OUT_DIR)/build

########################################################################################################################
# Phony
########################################################################################################################

.PHONY: default all clean toolchain

default: all

all: $(BIN_DIR)/virtdbg.bin

########################################################################################################################
# Targets
########################################################################################################################

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:%.o=%.d)
BINS ?=
-include $(DEPS)

$(BUILD_DIR)/virtdbg.elf: $(BINS) $(OBJS)
	@echo LD $@
	@mkdir -p $(@D)
	@$(LD) $(CFLAGS) -o $@ $(OBJS)

$(BIN_DIR)/virtdbg.bin: $(BUILD_DIR)/virtdbg.elf
	@echo OBJCOPY $@
	@mkdir -p $(@D)
	@$(OBJCOPY) -O binary -S \
		-j .init -j .text -j .data -j .bss -j .rodata\
		--set-section-flags .bss=alloc,load,contents\
		$^ $@

$(BUILD_DIR)/%.c.o: %.c
	@echo CC $@
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -MMD -D__FILENAME__="\"$<\"" -D__MODULE__="\"$(notdir $(basename $<))\"" -c $< -o $@

$(BUILD_DIR)/%.asm.o: %.asm
	@echo NASM $@
	@mkdir -p $(@D)
	@nasm -g -i $(BUILD_DIR) -F dwarf -f elf64 -o $@ $<

clean:
	rm -f artifacts/loader.elf
	make -C loader clean
	rm -rf out

########################################################################################################################
# Setup image
########################################################################################################################

QEMU_ARGS += -m 4G -smp 4
QEMU_ARGS += -machine q35
QEMU_ARGS += -serial file:/dev/stdout
QEMU_ARGS += -monitor stdio
QEMU_ARGS += --no-shutdown -d int
QEMU_ARGS += --no-reboot
QEMU_ARGS += -cpu host --enable-kvm
QEMU := qemu-system-x86_64

#
# A target to start the kernel in qemu
#
qemu: $(BIN_DIR)/image.hdd
	$(QEMU) -hdd $^ $(QEMU_ARGS)

#
# A target to build a bootable image
#
image: $(BIN_DIR)/image.hdd

artifacts/loader.elf:
	make -C loader all
	cp loader/stivale2.elf artifacts/loader.elf

#
# Builds the image itself
#
$(BIN_DIR)/image.hdd: \
		$(BIN_DIR)/virtdbg.bin \
		artifacts/limine.cfg \
		artifacts/loader.elf
	@mkdir -p $(@D)
	@echo "Creating disk"
	@rm -rf $@
	dd if=/dev/zero bs=1M count=0 seek=64 of=$@
	@echo "Creating echfs partition"
	parted -s $@ mklabel msdos
	parted -s $@ mkpart primary 1 100%
	echfs-utils -m -p0 $@ quick-format 32768
	@echo "Importing files"
	echfs-utils -m -p0 $@ import $(BIN_DIR)/virtdbg.bin virtdbg.bin
	echfs-utils -m -p0 $@ import artifacts/loader.elf loader.elf
	echfs-utils -m -p0 $@ import artifacts/limine.cfg limine.cfg
	@echo "Installing limine"
	artifacts/limine-install artifacts/limine.bin $@
