NAME    = firmware
SOURCES = startup.c main.c gpio.c syscon.c utils.c

OBJ = $(SOURCES:%.c=%.o)
CC      = arm-none-eabi-gcc
LD      = arm-none-eabi-ld
OC      = arm-none-eabi-objcopy
CCFLAGS  = '-mcpu=cortex-m3' -mthumb
LDFLAGS = '-Tlpc1343.ld' -nostartfiles -nostdlib -nodefaultlibs
OCFLAGS = -Obinary --strip-unneeded

all: $(NAME).bin

$(NAME).bin: $(NAME).hex
	cp $(NAME).hex $(NAME).bin
	checksum $(NAME).bin

$(NAME).hex: $(NAME).out
	$(OC) $(OCFLAGS) $(NAME).out $(NAME).hex
	
$(NAME).out: $(OBJ)
	$(LD) $(LDFLAGS) -o $(NAME).out $(OBJ)

%.o: %.c
	$(CC) $(CCFLAGS) -c $<

install: $(NAME).bin
	cp $(NAME).bin /Volumes/CRP\ DISABLD/
	diskutil umount /Volumes/CRP\ DISABLD/
	
clean:
	rm $(NAME).bin $(NAME).hex $(NAME).out $(OBJ)
