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
KERN_OBJFILES := swtch.o syscall.o proc.o panic.o scheduler.o timer.o memory.o console.o kbd.o vectors.o trapasm.o printfmt.o trap.o video.o string.o entry.o init.o hankaku.o

initcode: initcode.S
	$(CC) $(CFLAGS) -nostdinc -I. -c initcode.S
	$(LD) $(LDFLAGS) -N -e start -Ttext 0 -o initcode.out initcode.o
	$(OBJCOPY) -S -O binary initcode.out initcode
	$(OBJDUMP) -S initcode.o > initcode.asm

hankaku.o: hankaku.bin
	i386-elf-objcopy -I binary -O elf32-i386 -B i386 hankaku.bin hankaku.o

%.o: %.S
	@echo + as $<
	$(V)$(CC) -nostdinc $(KERN_CFLAGS) -c -o $@ $<

%.o: %.c
	@echo + cc $<
	@mkdir -p $(@D)
	$(V)$(CC) -nostdinc $(KERN_CFLAGS) -c -o $@ $<

kernel: $(KERN_OBJFILES) initcode kernel.ld
	@echo + ld $@
	$(V)$(LD) -o $@ $(KERN_LDFLAGS) $(KERN_OBJFILES) $(GCC_LIB) -b binary initcode

kernel.img: boot kernel
	@echo + mk $@
	$(V)dd if=/dev/zero of=kernel.img~ count=10000 2>/dev/null
	$(V)dd if=boot of=kernel.img~ conv=notrunc 2>/dev/null
	$(V)dd if=kernel of=kernel.img~ seek=1 conv=notrunc 2>/dev/null
	$(V)mv kernel.img~ kernel.img

QEMUOPTS = -drive file=kernel.img,index=0,media=disk,format=raw -serial mon:stdio -m 256M
IMAGES = kernel.img

GDBPORT = $(shell expr `id -u` % 5000 + 25001)

QEMUGDB = $(shell if $(QEMU) -help | grep -q '^-gdb'; \
	then echo "-gdb tcp::$(GDBPORT)"; \
	else echo "-s -p $(GDBPORT)"; fi)

qemu: all
	$(QEMU) $(QEMUOPTS)

.gdbinit: .gdbinit.tmpl
	sed "s/localhost:1234/localhost:$(GDBPORT)/" < $^ > $@

qemu-gdb: all .gdbinit
	@echo "*** Now run 'gdb'." 1>&2
	$(QEMU) $(QEMUOPTS) -S $(QEMUGDB)

gdb:
	i386-elf-gdb -n -x .gdbinit

clean:
	rm *.img
	rm *.out
	rm *.asm
	rm *.o
	rm .gdbinit
