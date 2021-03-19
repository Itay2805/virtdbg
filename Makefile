########################################################################################################################
# Build config
########################################################################################################################

#
# The cross compiler to use
#
TOOLCHAIN ?=

########################################################################################################################
# Build constants
########################################################################################################################

CC 			:= $(TOOLCHAIN)gcc
LD			:= $(TOOLCHAIN)gcc

CFLAGS 		:= -Wall -Werror -Wno-unused-label
CFLAGS 		+= -mno-sse -mno-sse2 -mno-mmx -mno-80387 -m64
CFLAGS 		+= -mno-red-zone -fno-builtin -march=nehalem
CFLAGS 		+= -ffreestanding -fno-asynchronous-unwind-tables
CFLAGS 		+= -Os -flto -ffat-lto-objects -g
CFLAGS 		+= -fPIC -fpie -mcmodel=large
CFLAGS 		+= -Ivirtdbg -shared -Wl,--omagic

CFLAGS 		+= -nostdlib -nodefaultlibs -nostartfiles
CFLAGS 		+= -z max-page-size=0x1000

SRCS		:= $(shell find virtdbg -name '*.c')

OUT_DIR 	:= out

BIN_DIR := $(OUT_DIR)/bin
BUILD_DIR := $(OUT_DIR)/build

########################################################################################################################
# Phony
########################################################################################################################

.PHONY: default all clean toolchain

default: all

all: $(BIN_DIR)/virtdbg.elf

########################################################################################################################
# Targets
########################################################################################################################

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:%.o=%.d)
BINS ?=
-include $(DEPS)

$(BIN_DIR)/virtdbg.elf: $(BINS) $(OBJS)
	@echo LD $@
	@mkdir -p $(@D)
	@$(LD) $(CFLAGS) -o $@ $(OBJS)

$(BUILD_DIR)/%.c.o: %.c
	@echo CC $@
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -MMD -D__FILENAME__="\"$<\"" -D__MODULE__="\"$(notdir $(basename $<))\"" -c $< -o $@

$(BUILD_DIR)/%.asm.o: %.asm
	@echo NASM $@
	@mkdir -p $(@D)
	@nasm -g -i $(BUILD_DIR) -F dwarf -f elf64 -o $@ $<

clean:
	rm -rf out

########################################################################################################################
# Setup image
########################################################################################################################

QEMU_ARGS += -m 4G -smp 4
QEMU_ARGS += -machine q35
QEMU_ARGS += -serial stdio
QEMU_ARGS += -monitor tcp:localhost:1337,nowait
QEMU_ARGS += --no-shutdown
QEMU_ARGS += --no-reboot
QEMU_ARGS += -cpu Nehalem

ifeq ($(shell uname -r | sed -n 's/.*\( *Microsoft *\).*/\1/p'), Microsoft)
	QEMU := qemu-system-x86_64.exe
	ifeq ($(QEMU_ACCEL), 1)
		QEMU_ARGS += --accel whpx
	endif
else
	QEMU := qemu-system-x86_64
	ifeq ($(QEMU_ACCEL), 1)
		QEMU_ARGS += --enable-kvm
	endif
endif

#
# A target to start the kernel in qemu
#
qemu: $(BIN_DIR)/image.hdd
	$(QEMU) -hdd $^ $(QEMU_ARGS)

#
# A target to build a bootable image
#
image: $(BIN_DIR)/image.hdd

#
# Builds the image itself
#
$(BIN_DIR)/image.hdd: \
		$(BIN_DIR)/virtdbg.bin \
		artifacts/limine.cfg \
		artifacts/lyre.elf
	@mkdir -p $(@D)
	@echo "Creating disk"
	@rm -rf $@
	dd if=/dev/zero bs=1M count=0 seek=64 of=$@
	@echo "Creating echfs partition"
	parted -s $@ mklabel msdos
	parted -s $@ mkpart primary 1 100%
	echfs-utils -m -p0 $@ quick-format 32768
	@echo "Importing files"
	echfs-utils -m -p0 $@ import $(BIN_DIR)/virtdbg.elf virtdbg.elf
	echfs-utils -m -p0 $@ import artifacts/lyre.elf lyre.elf
	echfs-utils -m -p0 $@ import artifacts/limine.cfg limine.cfg
	@echo "Installing limine"
	artifacts/limine-install artifacts/limine.bin $@
