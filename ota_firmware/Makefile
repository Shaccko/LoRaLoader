ARMCFLAGS ?=  -W -Wall -Wextra -Wundef -Wshadow -Wdouble-promotion \
            -Wformat-truncation -fno-common -Wconversion -I. \
	    -ffunction-sections -fdata-sections \
	    -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS ?= -W -Wall -Wextra -Wundef -Wshadow -Wdouble-promotion \
	-Wformat-truncation -fno-common -Wconversion -I.
LDFLAGS ?= -nostartfiles -nostdlib --specs nano.specs -lc -lgcc -Wl,--gc-sections -Wl,-Map=$@.map

FIRMWARE_LD ?= f411re.ld
FIRMWARE_SOURCES = stm32_ota_main.c rcc.c startup_f411re.c uart.c
FIRMWARE_HEADER = rcc.h hal.h uart.h 
FIRMWARE_ADDR = 0x0800C000

ifeq ($(OS),WINDOWS_NT)
	RM = cmd /C del /Q /F *.elf *~ *.o *.bin ota_upload_raspi
else
	RM = rm -f *.bin *.elf *.o ota_upload_raspi *.map
endif

build: firmware_ota.bin 

flash: firmware_ota.bin
	st-flash --reset write $< $(FIRMWARE_ADDR)

firmware_ota.bin: firmware_ota.elf
	arm-none-eabi-objcopy -O binary $< $@

firmware_ota.elf: $(FIRMWARE_SOURCES) $(FIRMWARE_HEADER)
	arm-none-eabi-gcc $(FIRMWARE_SOURCES) $(ARMCFLAGS) -T $(FIRMWARE_LD) $(LDFLAGS) -o $@

clean:
	$(RM)
