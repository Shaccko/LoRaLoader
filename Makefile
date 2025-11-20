BOOT_SRC := bootloader.c
BOOT_LD := bootloader.ld
BOOT_ELF := bootloader.elf

APP_SRC := startup_f411re.c main.c
APP_LD := f411re.ld
APP_ELF := app.elf

CC := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
CFLAGS := -mcpu=cortex-m4 -mthumb -O2 -ffreestanding -Wall -Wextra -I.
LDFLAGS := -nostdlib -T

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

bootloader: $(BOOT_ELF)

$(BOOT_ELF): $(BOOT_SRC:.c=.o)
	$(CC) $(CFLAGS) $(LDFLAGS) $(BOOT_LD) $^ -o $@

app: $(APP_ELF)
	
$(APP_ELF): $(APP_SRC:.c=.o)
	$(CC) $(CFLAGS) $(LDFLAGS) $(APP_LD) $^ -o $@


clean:
	rm -f *.o *.elf
