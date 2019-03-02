GCCPREFIX='i386-elf-'
V = @

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

BOOT_OBJS := boot.o

%.o: %.S
	@echo + as $<
	$(V)$(CC) -nostdinc $(KERN_CFLAGS) -c -o $@ $<

boot: $(BOOT_OBJS)
	$(V)$(LD) $(LDFLAGS) -N -e start -Ttext 0x7C00 -o $@.out $^
	$(V)$(OBJDUMP) -S $@.out >$@.asm
	$(V)$(OBJCOPY) -S -O binary -j .text $@.out $@
	$(V)perl sign.pl boot
	mv boot boot.img

QEMUOPTS = -drive file=boot.img,index=0,media=disk,format=raw -serial mon:stdio
IMAGES = boot.img

qemu:
	$(QEMU) $(QEMUOPTS)
