BOOT_SRC := bootloader.c rcc.c 
BOOT_OBJS := $(BOOT_SRC:.c=.o)
BOOT_ELF := bootloader.elf
BOOT_LD := bootloader.ld

APP_SRC := startup_f411re.c main.c rcc.c uart.c syscalls.c
APP_OBJS := $(APP_SRC:.c=.o)
APP_ELF := app.elf
APP_LD := f411re.ld

CC := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
CFLAGS ?= -W -Wall -Wextra -Werror -Wundef -Wshadow -Wdouble-promotion \
	  -Wformat-truncation -fno-common -Wconversion \
	  -g3 -Os -ffunction-sections -fdata-sections -I. \
	  -mcpu=cortex-m4 -mthumb -mfloat-abi=softfp -mfpu=fpv4-sp-d16 

LDFLAGS ?= -nostartfiles -nostdlib --specs nano.specs -lc -lgcc \
	   -Wl,--gc-sections -Wl,-Map=$@.map 

build: app.elf bootloader.elf

flash: bootloader.bin app.bin
	st-flash --reset write app.bin 0x08004000
	st-flash --reset write bootloader.bin 0x08000000

app.bin: app.elf
	$(OBJCOPY) -O binary $< $@

app.elf: $(APP_OBJS)
	$(CC) $(APP_OBJS) $(LDFLAGS) -T $(APP_LD) -o $@

bootloader.bin: bootloader.elf
	$(OBJCOPY) -O binary $< $@

bootloader.elf: $(BOOT_OBJS)
	$(CC) $(BOOT_OBJS) $(LDFLAGS) -T $(BOOT_LD) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	cmd /C del /Q /F *.elf *.o *.map *.bin
	
