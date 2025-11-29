ARMCFLAGS ?=  -W -Wall -Wextra -Wundef -Wshadow -Wdouble-promotion \
            -Wformat-truncation -fno-common -Wconversion -I. \
	    -ffunction-sections -fdata-sections \
	    -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS ?= -W -Wall -Wextra -Wundef -Wshadow -Wdouble-promotion \
	-Wformat-truncation -fno-common -Wconversion -I.
LDFLAGS ?= -nostartfiles -nostdlib --specs nano.specs -lc -lgcc -Wl,--gc-sections -Wl,-Map=$@.map

BOOTLOADER_LD ?= bootloader.ld
BOOTLOADER_SOURCES = bootloader.c startup_bootloader.c rcc.c syscalls.c uart.c 
BOOTLOADER_HEADER = rcc.h hal.h uart.h

FIRMWARE_LD ?= f411re.ld
FIRMWARE_SOURCES = stm32_main.c rcc.c startup_f411re.c uart.c syscalls.c spi_stm32.c LoRa_stm32.c
FIRMWARE_HEADER = rcc.h hal.h uart.h spi_stm32.h LoRa_stm32.h

RASPI_SOURCES = main_raspi.c gpio_raspi.c LoRa_raspi.c spi_raspi.c
RASPI_HEADERS = gpio_raspi.h LoRa_raspi.h spi_raspi.h


build: firmware.elf bootloader.elf firmware.bin bootloader.bin ota_upload_raspi

ota_upload_raspi: $(RASPI_SOURCES) $(RASPI_HEADERS)
	gcc $(RASPI_SOURCES) $(CFLAGS) -o $@

flash: bootloader.bin firmware.bin
	st-flash --reset write firmware.bin 0x08004000
	st-flash write bootloader.bin 0x08000000

firmware.bin: firmware.elf
	arm-none-eabi-objcopy -O binary $< $@

bootloader.bin: bootloader.elf
	arm-none-eabi-objcopy --pad-to=0x4000 --gap-fill=0x200 -O binary $< $@

firmware.elf: $(FIRMWARE_SOURCES) $(FIRMWARE_HEADER)
	arm-none-eabi-gcc $(FIRMWARE_SOURCES) $(ARMCFLAGS) -T $(FIRMWARE_LD) $(LDFLAGS) -o $@

bootloader.elf: $(BOOTLOADER_SOURCES) $(BOOTLOADER_HEADER)
	arm-none-eabi-gcc $(BOOTLOADER_SOURCES) $(ARMCFLAGS) -T $(BOOTLOADER_LD) $(LDFLAGS) -o $@

clean:
	rm -f *.o *.elf *.bin *.map spi_raspi
