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


void appStart(void) __attribute__ ((naked));  //__attribute__((noreturn)) ;
int main(void) __attribute__ ((OS_main,section (".init9"),externally_visible));


int main(void)
{
  uint8_t reset_reason = 0;

	/* here we learn how we were reset */
	reset_reason = MCUSR;
	MCUSR = 0;

	/* stop watchdog */
	WDTCSR |= _BV(WDCE) | _BV(WDE);
	WDTCSR = 0;

  /*
#ifdef BOOT_TOGGLE  // eeprom flag forces bootloader to run
  uint8_t toggle; READ_EEPROM(toggle, EEPROM_TOGGLE_ADDR)
	if ( (reset_reason & _BV(EXTRF)) || (reset_reason & _BV(PORF)) || toggle == 0 ) {
#else
	if ( (reset_reason & _BV(EXTRF)) || (reset_reason & _BV(PORF)) ) {      // If external reset
#endif
  */

	if ( (reset_reason & _BV(EXTRF)) || (reset_reason & _BV(PORF)) ) {      // If external reset

    stk500v1();

#ifdef MMC_CS
    load_eeprom();
#endif

	/* we reset via watchdog in order to reset all the registers to their default values */
	WDTCSR = _BV(WDE);
	while (1); // 16 ms

  } else {
    //load_eeprom();
    app_start();
  }
}


void toggle_clear() {
  WRITE_EEPROM(EEPROM_TOGGLE_ADDR, 0xff)
}

void load_eeprom(void) {
#ifdef BOOT_TOGGLE
  uint8_t toggle; READ_EEPROM(toggle, EEPROM_TOGGLE_ADDR)
  if (toggle == 0) {
    mmc_updater();

    // clear flag to avoid oo
    toggle_clear();
  }
#else
  mmc_updater();
#endif
}


void app_start(void) {
  //SP=RAMEND;
  __asm__ __volatile__ (
    // Jump to RST vector
    "clr r30\n"
    "clr r31\n"
    "ijmp\n"
  );
}


// Include a jumptable at top of flash so application
// can call bootloader routines

//#pragma GCC push_options
//#pragma GCC optimize ("no-lto")

asm (	".section .jumps,\"ax\",@progbits\n"
		".global _jumptable\n"
		"_jumptable:\n"
		"rjmp write_flash_page\n"
		"rjmp mmc_updater\n"); 

//__attribute__((optimize("no-lto")));

//#pragma GCC optimize ("lto")
//#pragma GCC pop_options
//

/* end of file board-stalker3.c */
