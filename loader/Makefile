TARGET := stivale2.elf

CC = cc

CFLAGS = -Wall -Wextra -O2 -pipe -g3

LDINTERNALFLAGS := \
	-Tlinker.ld    \
	-static        \
	-nostdlib      \
	-no-pie

INTERNALCFLAGS  :=       \
	-I.                  \
	-ffreestanding       \
	-Wno-unused-parameter\
	-fno-stack-protector \
	-fno-pic             \
	-mno-80387           \
	-mno-mmx             \
	-mno-3dnow           \
	-mno-sse             \
	-mno-sse2            \
	-mcmodel=kernel      \
	-Werror              \
	-mno-red-zone

CFILES := $(shell find ./ -type f -name '*.c')
OBJ    := $(CFILES:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LDINTERNALFLAGS) $(OBJ) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INTERNALCFLAGS) -c $< -o $@

clean:
	rm -rf $(TARGET) $(OBJ)
