#!/bin/bash

ISPTOOL="avrispmkII"
ISPTOOL="usbasp"
ISPPORT="usb"
ISPSPEED="-b 115200"


BOARD=`basename $1 | cut -d - -f 2`
MCU_TARGET=`basename $1 | cut -d - -f 3`

case "${MCU_TARGET}" in


"atmega168" )
EFUSE=00
HFUSE=DD
LFUSE=FF
;;
"atmega328p")
# stalker v3, save eeprom across flash, 4k bootloader
LFUSE=FF
#HFUSE=DF # dont save
HFUSE=D7 # save eeprom
EFUSE=FE

# stalker v3, save eeprom across flash
#LFUSE=FF
#HFUSE=D2
#EFUSE=FE
# stalker v3
#LFUSE=FF
#HFUSE=DA
#EFUSE=FE

# WORKS....
#LFUSE=FF
#HFUSE=D8
#EFUSE=FE

LFUSE=FF
HFUSE=D0
EFUSE=FE

;;
"atmega1280")
EFUSE=F5
HFUSE=DA
LFUSE=FF
;;
esac


avrdude -c ${ISPTOOL} -p ${MCU_TARGET} -P ${ISPPORT} ${ISPSPEED} -e -u -U lock:w:0x3f:m -U efuse:w:0x${EFUSE}:m -U hfuse:w:0x${HFUSE}:m -U lfuse:w:0x${LFUSE}:m
sleep 3
avrdude -c ${ISPTOOL} -p ${MCU_TARGET} -P ${ISPPORT} ${ISPSPEED} -U flash:w:$1 -U lock:w:0x0f:m
