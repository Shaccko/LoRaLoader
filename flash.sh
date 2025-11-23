#!/bin/bash

make
cat bootloader.bin firmware.bin > app.bin
st-flash --reset write 0x08000000 app.bin
