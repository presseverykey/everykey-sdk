NAME     = firmware
SOURCES  = $(shell find -L . -type f -name '*.c')
INCLUDES = $(shell find -L . -type f -name '*.h')
DEPENDS  = $(SOURCES:%.c=%.dep)
OBJS     = $(SOURCES:%.c=%.o)

CC       = arm-none-eabi-gcc
LD       = arm-none-eabi-ld
OC       = arm-none-eabi-objcopy
CCFLAGS  = '-mcpu=cortex-m3' '-mthumb'
LDFLAGS  = '-Tlpc1343.ld' -nostartfiles -nostdlib -nodefaultlibs
OCFLAGS  = -Obinary --strip-unneeded

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
$(NAME).obj: $(OBJS)
	$(LD) $(LDFLAGS) -o $(NAME).obj $(OBJS)

# compile .c to .o files. Generate .dep files on the fly.
%.o: %.c
	$(CC) $(CCFLAGS) -MD -o $@ -c $<
	@cp $*.d $*.dep; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
		-e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.dep; \
	rm -f $*.d

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

# clean all generated files
.PHONY: clean
clean:
	-rm $(NAME).bin $(NAME).hex $(NAME).obj $(OBJS) $(DEPENDS)
