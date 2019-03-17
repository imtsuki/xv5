GCCPREFIX='i386-elf-'
V = 

QEMU = qemu-system-i386

CC := $(GCCPREFIX)gcc -pipe
AS := $(GCCPREFIX)as
AR := $(GCCPREFIX)ar
LD := $(GCCPREFIX)ld
OBJCOPY := $(GCCPREFIX)objcopy
OBJDUMP := $(GCCPREFIX)objdump
NM := $(GCCPREFIX)nm

# Native commands
NCC := gcc $(CC_VER) -pipe
NATIVE_CFLAGS := $(CFLAGS) $(DEFS) $(LABDEFS) -I$(TOP) -MD -Wall
TAR := gtar
PERL := perl

CFLAGS := $(CFLAGS) $(DEFS) $(LABDEFS) -O1 -fno-builtin -I$(TOP) -MD
CFLAGS += -fno-omit-frame-pointer
CFLAGS += -std=gnu99
CFLAGS += -static
CFLAGS += -Wall -Wno-format -Wno-unused -Werror -gstabs -m32
CFLAGS += -fno-tree-ch
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

LDFLAGS := -m elf_i386

KERN_CFLAGS := $(CFLAGS) -DJOS_KERNEL -gstabs
USER_CFLAGS := $(CFLAGS) -DJOS_USER -gstabs

GCC_LIB := $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)

OBJDIR := obj

BOOT_OBJS := boot.o main.o

all: kernel.img

boot.o: boot.S
	@echo + as $<
	$(V)$(CC) -nostdinc $(KERN_CFLAGS) -c -o $@ $<

main.o: main.c
	@echo + cc -Os $<
	$(V)$(CC) -nostdinc $(KERN_CFLAGS) -Os -c -o main.o main.c

boot: $(BOOT_OBJS)
	@echo + ld $<
	$(V)$(LD) $(LDFLAGS) -N -e entry -Ttext 0x7C00 -o $@.out $^
	$(V)$(OBJDUMP) -S $@.out >$@.asm
	$(V)$(OBJCOPY) -S -O binary -j .text $@.out $@
	$(V)perl sign.pl boot


KERN_LDFLAGS := $(LDFLAGS) -T kernel.ld -nostdlib
KERN_SRCFILES := entry.c
KERN_OBJFILES := kbd.o vectors.o trapasm.o printfmt.o trap.o video.o string.o entry.o hankaku.o

%.o: %.S
	@echo + as $<
	$(V)$(CC) -nostdinc $(KERN_CFLAGS) -c -o $@ $<

%.o: %.c
	@echo + cc $<
	@mkdir -p $(@D)
	$(V)$(CC) -nostdinc $(KERN_CFLAGS) -c -o $@ $<

kernel: $(KERN_OBJFILES) kernel.ld
	@echo + ld $@
	$(V)$(LD) -o $@ $(KERN_LDFLAGS) $(KERN_OBJFILES) $(GCC_LIB)

kernel.img: boot kernel
	@echo + mk $@
	$(V)dd if=/dev/zero of=kernel.img~ count=10000 2>/dev/null
	$(V)dd if=boot of=kernel.img~ conv=notrunc 2>/dev/null
	$(V)dd if=kernel of=kernel.img~ seek=1 conv=notrunc 2>/dev/null
	$(V)mv kernel.img~ kernel.img

QEMUOPTS = -drive file=kernel.img,index=0,media=disk,format=raw -serial mon:stdio
IMAGES = kernel.img

qemu:
	$(QEMU) $(QEMUOPTS)

clean:
	rm *.img
	rm *.out
	rm *.asm
