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

/* function prototype */
void main (void) __attribute__ ((naked,section (".init9"),externally_visible));

/* some variables */
const void (*app_start)(void) = 0x0000;
uint8_t reset_reason = 0;

/* main program starts here */
void main(void)
{
	/* here we learn how we were reset */
	reset_reason = MCUSR;
	MCUSR = 0;

	/* stop watchdog */
	WDTCSR |= _BV(WDCE) | _BV(WDE);
	WDTCSR = 0;

#ifdef BOOT_TOGGLE  // eeprom flag forces bootloader to run
  int8_t toggle; READ_EEPROM(toggle, EEPROM_TOGGLE_ADDR)
	if ( (reset_reason & _BV(EXTRF)) || (reset_reason & _BV(PORF)) || toggle != -1 ) {
#else
	if ( (reset_reason & _BV(EXTRF)) || (reset_reason & _BV(PORF)) ) {      // If external reset
#endif
     
    /* this is needed because of the __attribute__ naked, section .init 9 */
    /* from now, we can call functions :-) */
    asm volatile ( "clr __zero_reg__" );
    SP=RAMEND;

    stk500v1();

#ifdef MMC_CS
    mmc_updater();
#endif

#ifdef BOOT_TOGGLE
    // clear flag to avoid oo
    //WRITE_EEPROM(EEPROM_TOGGLE_ADDR, 0xff)
    if (toggle != -1)
      WRITE_EEPROM(EEPROM_TOGGLE_ADDR, -1)
    //WRITE_EEPROM(EEPROM_TOGGLE_ADDR, toggle-1)
#endif

    /* we reset via watchdog in order to reset all the registers to their default values */
    WDTCSR = _BV(WDE);
    while (1); // 16 ms

  } else {
    app_start();
  }
}

/* end of file board-stalker3.c */
