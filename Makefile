########################################################################################################################
# Build config
########################################################################################################################

########################################################################################################################
# Build constants
########################################################################################################################

CC 			:= ./toolchain/bin/x86_64-elf-gcc
LD			:= ./toolchain/bin/x86_64-elf-gcc
OBJCOPY		:= ./toolchain/bin/x86_64-elf-objcopy

CFLAGS 		:= -Wall -Werror -Wno-unused-label
CFLAGS 		+= -mno-sse -mno-sse2 -mno-mmx -mno-80387 -m64
CFLAGS 		+= -mno-red-zone -fno-builtin -march=nehalem
CFLAGS 		+= -ffreestanding -fpic
CFLAGS 		+= -O2 -flto -ffat-lto-objects -g
CFLAGS 		+= -Ivirtdbg

CFLAGS 		+= -nostdlib
CFLAGS 		+= -z max-page-size=0x1000
CFLAGS  	+= -Tvirtdbg/linker.ld

SRCS		:= $(shell find virtdbg -name '*.c')

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
# Toolchain
########################################################################################################################

toolchain:
	scripts/make_toolchain.sh "`realpath ./toolchain`" -j`nproc`

########################################################################################################################
# Targets
########################################################################################################################

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:%.o=%.d)
BINS ?=
-include $(DEPS)

$(BIN_DIR)/virtdbg.bin: $(BUILD_DIR)/virtdbg.elf
	@echo OBJCOPY $@
	@mkdir -p $(@D)
	@$(OBJCOPY) -O binary -S -j .init -j .text -j .data $^ $@

$(BUILD_DIR)/virtdbg.elf: $(BINS) $(OBJS)
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
