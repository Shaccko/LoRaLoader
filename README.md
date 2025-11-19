# LoRa
Communicating with LoRa using SPI through STM32F411RE.
This project is built on top of the Bare Metal Programming Guide for STM32 ARM MCUs
(https://github.com/cpq/bare-metal-programming-guide)

by Ghanim
ghanimj@hotmail.com

Design purpose of this was to learn bare-metal embedded programming
on the ARM chip through our STM32F411RE MCU board. A basic custom SPI
driver was written to communicate with a LoRa module which is initialized
with default chip settings.

This was a learning experience for me on how fast bare-metal 
programming can get and a crash course on bit manipulation
register access through memory, and how linker scripts and the 
Makefile toolchain is utilized to flash our board with code.

It's incredible especially after coming from CubeIDE how convenient bare-metal
can be for the simplest of projects, especially since we are writing
drivers without introducing any unecessary function/library overhead 
and just using the absolute essentials.

To make use of this, make sure LoRa is configured with the default settings:

    - 433 frequency
    - 125KHz bandwidth
    - Code rate of 4/5
    - 20 dB gain power
    - Spread factor of 7 (SF_7)
    - LoRa pin configurations:
            #define LORA_PORT 'B'
            #define CS_PIN (PIN_NUM(0))
            #define RST_PIN (PIN_NUM(5))
            #define IRQ_PIN (PIN_NUM(3))
            #define MOSI
    
    - SPI pin configurations:
            #define SPI1_PORT 'A'
            #define SCK1_PIN (PIN_NUM(5))
            #define NSS1_PIN (PIN_NUM(4))
            #define MOSI1_PIN (PIN_NUM(6))
            #define MISO1_PIN (PIN_NUM(7))
            
            (Of course keeping in mind we are using the F411RE Nucleo Board)

These pin configurations are hard-coded in spi.h and LoRa.h. 
use "git clone" to clone this project on your directory.

# Features:
Basic transmission and receiving is functional. 

# Future TO-DOs
A sleep mode should be configured for optimized current usage.
Precise tuning for how much distance user needs would optimize current usage more.

Overall, I'd say this project has made me appreciate low-level programming more, and
I'm no longer scared looking at bits and can make a above-average effort at understanding
bit/low-level code. 







