NAME     = firmware
CSOURCES  = $(shell find -L . -type f -name '*.c')
ASOURCES = $(shell find -L . -type f -name '*.S')
INCLUDES = $(shell find -L . -type f -name '*.h')
DEPENDS  = $(CSOURCES:%.c=%.dep)
COBJS     = $(CSOURCES:%.c=%.o)
AOBJS    = $(ASOURCES:%.S=%.o) 

AS       = arm-none-eabi-as 
CC       = arm-none-eabi-gcc
LD       = arm-none-eabi-ld
OC       = arm-none-eabi-objcopy
CCFLAGS  = '-mcpu=cortex-m3' '-mthumb' '-std=c99'
LDFLAGS  = '-Tlpc1343.ld' -nostartfiles -nostdlib -nodefaultlibs
OCFLAGS  = -Obinary --strip-unneeded

JLINK = jLinkExe
FLASH_START_ADDRESS = 0x00000000

# all generates our target bin
all: $(NAME).bin

# generate bin from hex by setting the correct checksum
$(NAME).bin: $(NAME).hex
	cp $(NAME).hex $(NAME).bin
	checksum $(NAME).bin

# generate hex from obj by objcopying from linked obj file
$(NAME).hex: $(NAME).obj
	$(OC) $(OCFLAGS) $(NAME).obj $(NAME).hex
	
# generate obj by linking all .o files
$(NAME).obj: $(COBJS) $(AOBJS)
	$(LD) $(LDFLAGS) -o $(NAME).obj $(COBJS) $(AOBJS)

# compile .c to .o files. Generate .dep files on the fly.
%.o: %.c
	$(CC) $(CCFLAGS) -MD -o $@ -c $<
	@cp $*.d $*.dep; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
		-e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.dep; \
	rm -f $*.d

# compile assembler files
%.o: %.S
	$(AS) $(ASFLAGS) -o $@ -c $<

# include depencies are defined in .dep files. Include them as rules here.
-include $(DEPENDS)

# install target builds bin, copies it to LPC and unmounts it.
mac-install: $(NAME).bin
	cp $(NAME).bin /Volumes/CRP\ DISABLD/
	diskutil umount /Volumes/CRP\ DISABLD/
	
# Flash the firmware using the simpleflash Python script
# https://raw.github.com/r0ket/r0ket/master/tools/bootloader/simpleflash
simpleflash: $(NAME).bin
	simpleflash $(NAME).bin

## Program device using JLINK
flash: rm-flash.jlink flash.jlink $(NAME).bin
	$(JLINK) flash.jlink

rm-flash.jlink:
	-rm -rf flash.jlink

flash.jlink:
	echo "device lpc1343\nr\nloadbin $(NAME).bin, $(FLASH_START_ADDRESS)\nr\ng\nexit\n" > flash.jlink


# clean all generated files
.PHONY: clean
clean:
	-rm $(NAME).bin $(NAME).hex $(NAME).obj $(COBJS) $(AOBJS) $(DEPENDS)
