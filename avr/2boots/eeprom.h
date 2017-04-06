/**********************************************************/
/* eeprom.c                                               */
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

#define EEPROM_FILENAME_ADDR  E2END-1
#define EEPROM_TOGGLE_ADDR    E2END

uint8_t get_eeprom(void *);
uint8_t check_eeprom_toggle();
void put_eeprom(void *, uint8_t);
#endif

