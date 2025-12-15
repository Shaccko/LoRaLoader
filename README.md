# LoRaLoader
Downloading firmware over-the-air via LoRa.

This project is once again built on top of the Bare Metal Programming Guide for STM32 ARM MCUs
(https://github.com/cpq/bare-metal-programming-guide)

By Ghanim
ghanimj@hotmail.com

This is probably my capstone for bare-metal MCU programming. It stemmed
from the fact that I was not happy with how frightingly little I knew
about linker scripts, was not happy with just copy pasting what the bare metal guide shows
and out of spite I decided I needed exposure therapy with working
with it straight up by making an OTA firmware downloader. 
Sadly even though I did learn lots about linker scripts, I didn't end up programming
too much linker stuff since we just simply ended up writing to flash
instead of dedicating a special region inside SRAM for OTA firmware.
Super fun project, with a small taste of system on chip programming. 

The programming timeline was as follows:

	- Programming a bootloader starting before main app, properly jumping to app
	- Figure out how to write RASPI user level drivers
	- Verify valid SPI signals using my oscilloscope
	- Configure receive reception for LoRa properly using external interrupts
	- Make sure both LoRa modules on MCU and RASPI are properly in sync
	- Find a way to read binary firmware files
	- Find a way for proper packet guarding using headers/checksums/chunk numbers synchronization
	- Make sure bytes are received in little-endian
	- Really get familiar with memory layout ARM MCUs
	- Finished

Logic errors were heinous to deal with, mistakes learned from this is to NOT write massive chunks
of code then spend hours debugging, deal with small chunks and scale up when familiar with tech
stack. 

# Devices used
Our transmitter is the RASPI 400 (a keyboard SoC Raspi), or just the Raspi 4. Consult BCM2835 ARM
peripheral for more info. Receiver is the STM32 F411RE, drivers used in this file are written keeping
ARM Cortex M4 in mind. Consult the RM0383 reference manual for more info.

# Bootloader
Our bootloader lives at 0x08000000 in sector 1. It's function is to check for the OTA magic byte
if it exists in SRAM, send an ACK back to RASPI, RASPI in turn starts sending in packets via LoRa
for MCU to program into flash region B. If no packet reception is cut or corrupt packets arrive,
sector 3 (flash B) is immediately erased, and we move to flash A instead. 

On the case where neither flash exists, we just run forever. If flash A/B is written correctly, 
we should expect the starting address of each flash region to hold the MSPs pointer 
in address, which is at 0x20020000, right on top of sram. To jump from bootloader 
to a flash region, we first disable interrupts, disable SYSTICK, obtain MSP and reset from 
0x0 and 0x0+4 of respective flash region, and finally we use some ASM to switch bootloader's 
MSP to either flash regions MSP, then call reset().

# Flash Programming
The F411RE has about 512k bytes of flash memory, and with it we are given
8 sectors of writable memory. I decided to use sector 3 for flash region B,
sectors 1-2 for main flash region A, and sector 0 for our bootloader, 16kb per sector
(except 4-7). STM32's reference sheet nicely outlines the essential steps in programming
to flash, we simply unlock flash, wait for BSY, set program/erase bit, write to flash, 
wait for BSY. I made sure to relock immediately after incase of electrical disturbances.

# Linker
Nothing much to say about linker programming, we just add another region for flash B and 
bootloader for each app, a symbol for start of OTA flash region, another symbol that holds
the OTA magic byte inside sram. I can, however, confidently say I can write my own linker
script from scratch. 

# OTA Packet Downloading
Initially, packets were being sent as data bufs which held metadata about packet validity
at [0]-[3], then [4]-[n] was actual binary file data. This changed to structs that held
metadata and binary file data. Same concept, just easier to handle and translate on receiver
side. 

ACKs on either side were used pretty liberally. On each packet deliver receiver sends an ACK back
to transmitter, if no ACK is sent transmitter tries one two three times, then fails and 
exits gracefully. If receiver is in the middle of packet reception, it starts a timer
of 5s every packet delivered, if timer goes off, receiver stops and boots to flash A. 
We make sure not to ever hang MCU anyhow if packets were failed, we erase the entire
sector and reset back to main app, where we then again wait for the OTA magic byte to arrive
and restart packet download if need be.

LoRa is obviously not the ideal packet delivery system. LoRa's FiFO has a hard limit of 255 bytes,
I was hard-coding it to 200 bytes just in case I somehow end up messing with LoRa hardware
by pushing it all the way to 250+4 bytes (binary data + metadata). Frequency had to be upped 5 fold
for faster packet delivery. A system where we send N amount of bytes, checksum till N bytes, then
send an ack back was in development, but logic ended up taking too long. This can significantly
increase packet reception without having to increase LoRa frequency by so much. Something of a feature
endeavour with this project for sure. For now, each individual packet is XOR checksum'd, its chunk
number compared on receiver and transmitter, and an ACK is sent back to transmitter if its valid.

# Project usage
Actually setting your own firmware to be downloaded is pretty straightforward, make sure 
the .bin firmware file exists, set its linker for it to be set at 0x0800C000 (sector 3),
make startup code set vector table to new firmware. 

	To download firmware:
	- Flash A must live at 0x08004000
	- Run ./ota_upload_firmware
	- Wait for Flash A to receive magic byte
	- Perform a system reset using NVIC_SystemReset() implementation or 
	using nucleo's reset button
	- As it starts up bootloader, OTA byte is detected, bootloader should
	send ACK back to RASPI.
	- Wait for packets to be downloaded
	- Bootloader jumps to Flash B if binary file is valid/no corrupted packets.

The pinout structure is again hard defined like previous projects:

	RASPI 4:
	LoRa pins:
	Hardware chip select is used instead of user CS.
	- GPIO 8 (SPI0_CE0_N)
	- #define RST_PIN (PIN_NUM(22)) GPIO22
		Interrupt wasn't configured for raspi, busy polling
		was preferred instead.

STM32 F411RE:

	LoRa pins:
	- #define LORA_PORT 'B'
	- #define CS_PIN (PIN_NUM(0))
	- #define RST_PIN (PIN_NUM(5))
	- #define IRQ_PIN (PIN_NUM(3))

	SPI pins:
	- #define SPI1_PORT 'A'
	- #define SCK1_PIN (PIN_NUM(5))
	- #define NSS1_PIN (PIN_NUM(4))
	- #define MOSI1_PIN (PIN_NUM(6))
	- #define MISO1_PIN (PIN_NUM(7))

# Features
	- OTA firmware download on a STM32 device

	- Software validity check on every packet received

	- Timeouts configured on each side to ensure no 
	program/CPU hangs/hardfaults are in place.

Incredibly fun project, on the same scale as my Thermostat
project on HAL CubeIDE. Super enlightening to work with registers
at a bare-metal level, and very memory space efficient, total binary 
.elf size of bootloader+flashA+flashB ended up being less than 10kb.

Some future improvements:
Definitely implement N bytes received validity checks, instead of checking
at every packet transmitted. This will significantly improve transmission/receive
rate. 

We could also use a capacitor placed at every LoRa's 3.3v connection, this will 
help with noise being picked up and sending garbage data, although software ignores
it and doesn't make it an issue anyways. 

