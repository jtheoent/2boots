/**********************************************************/
/* board-arduino.c                                        */
/* Copyright (c) 2010 by thomas seiler                    */
/* 2boots board file for arduino boards                   */
/* 2017 Changed boot sequence @jtheorent                  */
/* -------------------------------------------------------*/
/*                                                        */
/* This program is free software; you can redistribute it */
/* and/or modify it under the terms of the GNU General    */
/* Public License as published by the Free Software       */
/* Foundation; either version 2 of the License, or        */
/* (at your option) any later version.                    */
/*                                                        */
/* This program is distributed in the hope that it will   */
/* be useful, but WITHOUT ANY WARRANTY; without even the  */
/* implied warranty of MERCHANTABILITY or FITNESS FOR A   */
/* PARTICULAR PURPOSE.  See the GNU General Public        */
/* License for more details.                              */
/*                                                        */
/* You should have received a copy of the GNU General     */
/* Public License along with this program; if not, write  */
/* to the Free Software Foundation, Inc.,                 */
/* 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA */
/*                                                        */
/* Licence can be viewed at                               */
/* http://www.fsf.org/licenses/gpl.txt                    */
/**********************************************************/


/* some includes */
#include <inttypes.h>
#include <avr/io.h>
#include "stk500v1.h"
#include "mmc_fat.h"
#include "eeprom.h"

int main(void)       __attribute__ ((OS_main,section (".init9"),externally_visible,noreturn));
void appStart(void)  __attribute__((noreturn));
void load_name(void) __attribute__((externally_visible,noreturn)); // referenced in jumptable section
//void load_name(char *) __attribute__((externally_visible,noreturn)); // referenced in jumptable section
void load_eeprom(void);

int main(void)
{
  uint8_t reset_reason = 0;

	/* here we learn how we were reset */
	reset_reason = MCUSR;
	MCUSR = 0;

	/* stop watchdog */
	WDTCSR |= _BV(WDCE) | _BV(WDE);
	WDTCSR = 0;

	if ( (reset_reason & _BV(EXTRF)) || (reset_reason & _BV(PORF)) ) {      // If external reset

    asm("cli");
    stk500v1();

    /*
#ifdef MMC_CS
    load_eeprom();
#endif
    */

    /* we reset via watchdog in order to reset all the registers to their default values */
    //asm("sei");
    WDTCSR = _BV(WDE);
    while (1); // 16 ms
  }

  app_start();
  __builtin_unreachable();  // prove that we wont return
}

void toggle_clear() {
  WRITE_EEPROM(EEPROM_TOGGLE_ADDR, 0xff)
}


//void load_name(char *fname) {
void load_name() {

//void * __builtin_frame_address (unsigned int level)

  asm("cli");
	asm volatile ("clr __zero_reg__");

  /*
  // copy filename
  uint8_t i = 0;
  while (i++ < 11) {
    file_name[i] = *(fname++);
  }
  */

  /*
  char *i = FILENAME;
  while (i < (FILENAME+12)) {
    *(i++) = *(fname++);
  }
  */

  // reset stack to top of memory
  //SP = FILENAME - 1;
  SP = RAMEND;

  //mmc_updater();
  load_eeprom();

	WDTCSR = _BV(WDE);
	while (1); // 16 ms
  __builtin_unreachable();  // prove that we wont return
}


void load_eeprom(void) {
#ifdef BOOT_TOGGLE
  char c;
  uint8_t i = 0;


  //if ( *((uint8_t *)RAMEND) == 0) {
  READ_EEPROM(c, EEPROM_TOGGLE_ADDR)
  //if (c == 0) {

    /*
    // copy eeprom filename to ram
    for(i = 0; i< 11; i++) {
      READ_EEPROM(c, EEPROM_FILENAME_ADDR - i)
      f.name[i] = c;
    }
    */

    mmc_updater();

    // clear flag to avoid oo
    toggle_clear();
  //}
#else
  mmc_updater();
#endif

}

void app_start(void) {
  // Jump to RST vector
  __asm__ __volatile__ (
    "clr r30\n"
    "clr r31\n"
    "ijmp\n"
  );
  __builtin_unreachable();  // prove that we wont return
}

// Include a jumptable at top of flash so applications
// can call bootloader routines
asm (	".section .jumps,\"ax\",@progbits\n"
		".global _jumptable\n"
		"_jumptable:\n"
		"rjmp write_flash_page\n"
		"rjmp load_name\n"
    ".section .text\n"); 

/* end of file board-stalker3.c */
