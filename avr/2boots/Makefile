# 2boots Makefile
# (c) 2011-2017 thomas seiler      (thseiler@gmail.com)
# 

# 'all' is the default target: 
.PHONY: all 

# .hex files are considered intermediary files, but we want to keep them, so
# we declare secondary, which tells make to keep intermediary files...
.SECONDARY:

# setup avr-gcc to compile very space efficient
CC       = avr-gcc		# 6.2.0
CC       = /Applications/Arduino2.app/Contents/Java/hardware/tools/avr/bin/avr-gcc # arduino 1.8.1 => gcc 4.9.2
OBJCOPY  = avr-objcopy
OBJDUMP  = avr-objdump
OPTIMIZE = -mno-interrupts -funsigned-char -funsigned-bitfields -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -mrelax
AVRSIZE  = avr-size

override CFLAGS  = -x c $(OPTIMIZE) -g2 -Wall -mmcu=$(MCU) -DF_CPU=$(FCPU) $(DEFS) -std=gnu99 -flto
override LDFLAGS = -Wl,$(LDSECTION) -nostartfiles -nodefaultlibs -nostdlib -nostartfiles -Wl,--gc-sections -mrelax -mmcu=$(MCU) -flto -Os


# these are our actual source files
# the BOARD variable will be replaced later, depending on which board is beeing compiled
SRC  = $(BOARD).c stk500v1.c mmc_fat.c prog_flash.c
OBJS = $(BOARD).o stk500v1.o mmc_fat.o prog_flash.o

# the 'all' rule should build 2boots for all supported boards
# to create one target per board-*.c file we use a $(wildcard... ) rule
all: buildfolder $(subst .c,,$(wildcard board-*.c)) size-report 

# here we create the 'build' folder in case it does not exist
buildfolder:
	[ -d build ] || mkdir build

#each board can list all possible combinations for FCPUS, MCUS, and MMC_CS here
#these variables are target specific, so each board can define a different value
#(see http://www.gnu.org/software/make/manual/make.html#Target_002dspecific)
#
# To compile 2boot twice for a given board (i.e. two frequencies), simply
# separate the arguments with a space

#default for any unknown board
board-%: FCPUS = 16000000L
board-%: MCUS = atmega328p
board-%: MMC_CSS = nommc

#arduino
board-arduino: FCPUS = 8000000L 16000000L
board-arduino: MCUS = atmega168 atmega328p atmega1280
board-arduino: MCUS = atmega168 atmega328p
board-arduino: MMC_CSS = nommc PORTD4

#stalker:
board-stalker3: FCPUS = 8000000L
board-stalker3: MCUS = atmega328p
board-stalker3: MMC_CSS = PORTB2
board-stalker3: DEFS += -DBOOT_TOGGLE


# set certain variables per MCU

#atmega168:
2boots-atmega168%:  LDSECTION := --section-start=.text=0x3800

#atmega328p:
2boots-atmega328p%:  LDSECTION := --section-start=.text=0x7800 

#atmega1280:
2boots-atmega1280%:  LDSECTION := --section-start=.text=0x1F000
2boots-atmega1280%:  DEFS += -DLARGE_ADDR_SPACE


# This rule finally builds 2boots.
# The idea here is to learn the configuration from the make target name
# This is done with the help of good old shell (tm)
2boots-%: MCU =  $(shell echo $@ | cut -d - -f 2 )
2boots-%: FCPU = $(shell echo $@ | cut -d - -f 3 )
2boots-%: MMC_CS = $(shell echo $@ | cut -d - -f 4)
2boots-%: MMC_PORT=$(shell echo $(MMC_CS) | cut -c -5)
2boots-%: MMC_DDR=DDR$(shell echo $(MMC_PORT) | cut -c 5)
2boots-%: BOARD = $(shell echo $(subst .hex,,$@) | awk 'BEGIN {FS="-"} { for (f=5;f<=NF;f++) { printf($$f); if (f != NF) { printf("-") } } }' )
2boots-%: DEFS += $(if $(findstring nommc,$(MMC_CS)) , , -DMMC_CS=$(MMC_CS) -DMMC_PORT=$(MMC_PORT) -DMMC_DDR=$(MMC_DDR) ) 
2boots-%: ELFNAME = 2boots-$(subst board-,,$(BOARD))-$(MCU)-$(FCPU)-$(MMC_CS).elf
2boots-%: HEXNAME = 2boots-$(subst board-,,$(BOARD))-$(MCU)-$(FCPU)-$(MMC_CS).hex
2boots-%:
	@echo 
	@echo ----------[$@]-------------------
	@echo Compiling 2boots for: 
	@echo - $(BOARD) 
	@echo - MCU:$(MCU) @ $(subst 000L,,$(FCPU))kHz
	@echo - MMC_CS:$(MMC_CS) PORT:$(MMC_PORT) DDR:$(MMC_DDR)
	@echo
	@echo "#include \"$(BOARD).h\"" > board.h
#	This is a two step compile
#	$(CC) $(CFLAGS) $(SRC) -c 
#	$(CC) $(LDFLAGS) $(OBJS) -o build/$(ELFNAME) $(LIBS)
#	This is a one step compile
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRC) -o build/$(ELFNAME) $(LIBS)
	@echo
	@echo sucessfully compiled to build/$(ELFNAME)
	@$(AVRSIZE) build/$(ELFNAME)
	$(OBJCOPY) -j .text -j .data -O ihex build/$(ELFNAME) build/$(HEXNAME)	
	@echo 


# here come a few handy pattern rules for objcopy / objdump, 
# just in case someone wants a different format...

%.lst: %.elf
	$(OBJDUMP) -h -S build/$< > build/$@

%.hex: %.elf %.lst
	$(OBJCOPY) -j .text -j .data -O ihex build/$< build/$@	

%.srec: %.elf
	$(OBJCOPY) -j .text -j .data -O srec build/$< build/$@

%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -O binary build/$< build/$@

# this one is handy, too
clean:
	rm -rf build/*.elf build/*.lst build/*.srec build/*.bin build/*.hex

# this helps to keep track of used / free space
size-report:
	@echo -----[Size reports]----------------------------------------
	@echo 
	@$(AVRSIZE) build/*.elf

# This section finally creates targets for all combinations of boards, mcus, mmc-cs and frequencies:
# because of evil make magic (secondary expansion) it should be the last rule
board-%: TARGETS = $(foreach currentmcu, $(MCUS) , $(foreach currentfreq, $(FCPUS), $(foreach currentmmccs, $(MMC_CSS), 2boots-$(currentmcu)-$(currentfreq)-$(currentmmccs)-$@.hex ) ) )
.SECONDEXPANSION:
board-%:  $$(TARGETS) ;
