ARMCFLAGS ?=  -W -Wall -Wextra -Wundef -Wshadow -Wdouble-promotion \
            -Wformat-truncation -fno-common -Wconversion -I. \
	    -ffunction-sections -fdata-sections \
	    -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS ?= -W -Wall -Wextra -Wundef -Wshadow -Wdouble-promotion \
	-Wformat-truncation -fno-common -Wconversion -I.
LDFLAGS ?= -nostartfiles -nostdlib --specs nano.specs -lc -lgcc -Wl,--gc-sections -Wl,-Map=$@.map

BOOTLOADER_LD ?= bootloader.ld
BOOTLOADER_SOURCES = bootloader.c startup_bootloader.c rcc.c \
		     syscalls.c uart.c flash.c packet_parser.c \
		     spi_stm32.c
BOOTLOADER_HEADER = rcc.h hal.h uart.h spi_stm32.h packet_parser.h 
BOOTLOADER_ADDR = 0x08000000

FIRMWARE_LD ?= f411re.ld
FIRMWARE_SOURCES = stm32_main.c rcc.c startup_f411re.c uart.c \
		   syscalls.c spi_stm32.c sx1278_fsk.c exti.c \
		   packet_parser.c flash.c
FIRMWARE_HEADER = rcc.h hal.h uart.h spi_stm32.h sx1278_fsk.h \
		  exti.h packet_parser.h flash.h
FIRMWARE_ADDR = 0x08008000

FIRMWARE_OTA_SOURCES = stm32_new_main.c rcc.c startup_f411re.c uart.c
FIRMWARE_OTA_HEADER = rcc.h hal.h uart.h 

RASPI_SOURCES = main_raspi.c gpio_raspi.c spi_raspi.c \
		packet_transmitter.c sx1278_fsk.c
RASPI_HEADERS = gpio_raspi.h spi_raspi.h packet_transmitter.h \

ifeq ($(OS),WINDOWS_NT)
	RM = cmd /C del /Q /F *.elf *~ *.o *.bin ota_upload_raspi
else
	RM = rm -f *.bin *.elf *.o ota_upload_raspi *.map
endif

build: firmware.elf bootloader.elf firmware.bin bootloader.bin ota_upload_raspi

ota_upload_raspi: $(RASPI_SOURCES) $(RASPI_HEADERS)
	gcc $(RASPI_SOURCES) $(CFLAGS) -o $@

flash: firmware.bin
	st-flash --reset write $< $(FIRMWARE_ADDR)

flash-all: bootloader.bin firmware.bin
	st-flash --reset write firmware.bin $(FIRMWARE_ADDR) 
	st-flash write bootloader.bin $(BOOTLOADER_ADDR)

firmware_new.bin: firmware_new.elf
	arm-none-eabi-objcopy -O binary $< $@

firmware.bin: firmware.elf
	arm-none-eabi-objcopy -O binary $< $@

bootloader.bin: bootloader.elf
	arm-none-eabi-objcopy --pad-to=0x4000 --gap-fill=0x200 -O binary $< $@

firmware.elf: $(FIRMWARE_SOURCES) $(FIRMWARE_HEADER)
	arm-none-eabi-gcc $(FIRMWARE_SOURCES) $(ARMCFLAGS) -T $(FIRMWARE_LD) $(LDFLAGS) -o $@

bootloader.elf: $(BOOTLOADER_SOURCES) $(BOOTLOADER_HEADER)
	arm-none-eabi-gcc $(BOOTLOADER_SOURCES) $(ARMCFLAGS) -T $(BOOTLOADER_LD) $(LDFLAGS) -o $@

clean:
	$(RM)
