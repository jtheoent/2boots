
/**********************************************************/
/* mmc_fat.c                                              */
/* Copyright (c) 2010 by thomas seiler                    */ 
/* read a file from a FAT16 formatted MMC card            */
/* Code taken from HolgerBootloader (public domain)       */
/* from mikrokontroller.net and adapted for smaller size  */
/* SD support and code shrink by @jtheorent               */
/*                                                        */
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
#include <avr/io.h>
#include <inttypes.h>
#if !defined(__AVR_ATmega168__) || !defined(__AVR_ATmega328P__)
#include <avr/eeprom.h>  /* filename from eeprom */
#endif
#include "eeprom.h"

inline uint8_t get_eeprom(void *addr) {
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
		while(EECR & (1<<EEPE));
		EEAR = addr;
		EECR |= (1<<EERE);
		return EEDR;
#else
		return eeprom_read_byte(addr);
#endif
}

uint8_t check_eeprom_toggle() {
  return (get_eeprom(EEPROM_TOGGLE_ADDR) != 0xff);
}

void put_eeprom(void *addr, uint8_t byte) {
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
  while(EECR & (1<<EEPE));
  EEAR = addr;
  EEDR = byte;
  EECR |= (1<<EEMPE);
  EECR |= (1<<EEPE);
#else
  eeprom_write_byte(addr, byte);
#endif
}
