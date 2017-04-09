/**********************************************************/
/* eeprom.h                                               */
/* Copyright (c) 2017 by @jtheorent                       */ 
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
#ifndef _eeprom_h_
#define _eeprom_h_

#define EEPROM_TOGGLE_ADDR    E2END

#ifdef BOOT_TOGGLE
  #define EEPROM_FILENAME_ADDR  E2END-1
#else
  #define EEPROM_FILENAME_ADDR  E2END
#endif

#if defined(__AVR_ATmega168__)  || defined(__AVR_ATmega328P__)
  #define WRITE_EEPROM(_addr, _value) \
      while(EECR & (1<<EEPE)); \
 			EEAR = (uint16_t)(void *)_addr; \
 			EEDR = (uint8_t)_value; \
 			EECR |= (1<<EEMPE);\
 			EECR |= (1<<EEPE);
#else
  #define WRITE_EEPROM(_addr, _value) \
    eeprom_write_byte(_addr, _value);
#endif

#if defined(__AVR_ATmega168__)  || defined(__AVR_ATmega328P__)
  #define READ_EEPROM(_var, _addr) \
    while(EECR & (1<<EEPE)); \
    EEAR = (uint16_t)(void *)_addr; \
    EECR |= (1<<EERE); \
    _var = EEDR;
#else
  #define READ_EEPROM(_var,_addr) \
    _var = eeprom_read_byte((void *)_addr);
#endif


#endif
