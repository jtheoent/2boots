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
#include "board.h"
#include "mmc_fat.h"
#include "prog_flash.h"

#define true  1
#define false 0

#define SD_SUPPORT
//#define MMC_SUPPORT

#ifdef MMC_CS

/* ---[ SPI Interface ]---------------------------------------------- */

static void spi_send_byte(uint8_t data)
{
	SPDR=data;
	loop_until_bit_is_set(SPSR, SPIF); // wait for byte transmitted...
}


// delay 8 clocks, or get result
static void spi_send_ff(void)
{
  spi_send_byte(0xFF);
}


static uint8_t send_cmd(uint8_t cmd, uint32_t params, uint8_t crc)
{
	uint8_t i;
	uint8_t result;
	
	spi_send_ff();
	MMC_PORT &= ~(1<<MMC_CS); //MMC Chip Select -> Low (activate mmc)

  spi_send_byte(cmd);
  spi_send_byte(params >> 24);
  spi_send_byte(params >> 16);
  spi_send_byte(params >> 8);
  spi_send_byte(params);
  spi_send_byte(crc);

	// wait for response
	for (i=255;i;i--) {
	
 		spi_send_ff();
		result = SPDR;
		
		if ((result & 0x80) == 0)
      return(result);
	}

	return(result); // TimeOut?
}

/* ---[ MMC Interface ]---------------------------------------------- */

#define MMC_CMD0_RETRY	(uint8_t)16

// MMC Commands needed for reading a file from the card
#define MMC_GO_IDLE_STATE     0x40 +  0
#define MMC_SEND_OP_COND      0x40 +  1
#define MMC_CMD8              0x40 +  8
#define MMC_CMD55             0x40 + 55
#define MMC_ACMD41            0x40 + 41
#define MMC_CMD10             0x40 + 10
#define MMC_CMD58             0x40 + 58
#define MMC_READ_SINGLE_BLOCK 0x40 + 17

/* the sector buffer */
uint8_t buff[512];
uint8_t address_scale = 0;  // bits to shift address, 0 if block addressing, 9 for 512b MMC

/*			
*		Call mmc_init one time after a card has been connected to the µC's SPI bus!
*	
*		return values:
*			MMC_OK:				MMC initialized successfully
*			MMC_INIT:			Error while trying to reset MMC
*/
static inline uint8_t mmc_init(void)
{
	uint8_t i;
	uint8_t res;

	SPI_DDR  |= 1<<SPI_CLK | 1<<SPI_MOSI | 1<<SPI_SS; //SPI Data -> Output
	MMC_DDR |= 1<<MMC_CS;                             //MMC Chip Select -> Output

	SPCR = 1<<SPE | 1<<MSTR | SPI_INIT_CLOCK;         //SPI Enable, SPI Master Mode

	SPI_PORT |= 1<<SPI_SS;                            //PB2 output: High (deselect other SPI chips)
	

  // Pulse 80+ clocks to reset MMC
  for (i=10; i;i--)
    spi_send_ff();


  // CMD0
	for (i=MMC_CMD0_RETRY,res=0; i;i--) {
    res = send_cmd(MMC_GO_IDLE_STATE, 0, 0x95);
		if (res == 0x01)      //Response R1 from MMC (0x01: IDLE, The card is in idle state and running the initializing process.)
			break;
	}
	if (res != 0x01)
		return(MMC_INIT);

#ifdef SD_SUPPORT

  // CMD8
  if (res = send_cmd(MMC_CMD8, 0x01aa, 0x87) != 0x01)
    return(MMC_INIT);

  spi_send_ff();  // get 4-byte response
  spi_send_ff();
  spi_send_ff();
  spi_send_ff();


		//MMC_PORT |= 1<<MMC_CS; spi_send_ff();


  for (i=255; i;i--) {
    //while(1) {

    // CMD55
    if (res = send_cmd(MMC_CMD55, 0, 0x87) != 0x01)
      return(MMC_INIT);

      //MMC_PORT |= 1<<MMC_CS; spi_send_ff(); // delay


    // ACDM41

    res = send_cmd(MMC_ACMD41, 0x40100000, 0xcd);  // 0x40100000:0xcd SDHC 3.2-3.3v, 0x40000000:0x77, 0x00100000:0x5f SD 3.2-3.3v

      //MMC_PORT |= 1<<MMC_CS; spi_send_ff();


    //if (res == 0 || res == 5) break;
    if (res == 0 ) break;
  }

  // CMD58 CRC?
  // CMD10 set block length 512
  // res=send_cmd(MMC_CMD10, 0x200, 0xff);

#endif //SD_SUPPORT
#ifdef MMC_SUPPORT
  for (i=255; i;i--) {  // maybe too SHORT for some of cards!
    if (res=send_cmd(MMC_SEND_OP_COND, 0, 0) == 0)
      break;
    MMC_PORT |= 1<<MMC_CS; spi_send_ff(); spi_send_ff();
  }
  if (res) return MMC_INIT;
#endif

	SPCR = 1<<SPE | 1<<MSTR | SPI_READ_CLOCK; //SPI Enable, SPI Master Mode   // possibly comment out?
  return(MMC_OK);
}


static inline uint8_t wait_start_byte(void) {

	uint8_t i;
	
  for (i=255; i;i--) {
		spi_send_ff();
		if (SPDR == 0xFE) return MMC_OK;
  }
	return MMC_NOSTARTBYTE;
}

/*
 *		mmc_start_read_sector initializes the reading of a sector
 *
 *		Parameters:
 *			addr: specifies address to be read from
 *
 *		Return values:
 *			MMC_OK:						Command successful
 *			MMC_CMDERROR:			Error while sending read command to mmc
 *			MMC_NOSTARTBYTE:	No start byte received
 */
static uint8_t mmc_start_read_block(uint32_t addr)
{

  // MMC uses byte addressing, shift sectors by block size
	addr <<= address_scale;

	if (send_cmd(MMC_READ_SINGLE_BLOCK, addr, 0) != 0x00 || wait_start_byte()) {
		//MMC_PORT |= 1<<MMC_CS; //MMC Chip Select -> High (deactivate mmc);
		return(MMC_CMDERROR); //wrong response!
	}
	
	//mmc_read_buffer
	uint8_t  *buf;
	uint16_t len;
 
	buf = buff;
	len= 512;
	
	while (len) {
		spi_send_ff();
		*buf++ = SPDR;
		len--;
	}
	
	//read 2 bytes CRC (not used);
	spi_send_ff();
	spi_send_ff();
	MMC_PORT |= 1<<MMC_CS; //MMC Chip Select -> High (deactivate mmc);
	
	return(MMC_OK);
}


/* ---[ FAT16 ]------------------------------------------------------ */

static uint16_t  RootDirRegionStartSec;
static uint32_t  DataRegionStartSec;
static uint16_t  RootDirRegionSize;
static uint8_t   SectorsPerCluster;
static uint16_t  FATRegionStartSec;

static struct _file_s {
	uint16_t startcluster;
 	uint16_t sector_counter;
 	uint32_t size;
 	uint8_t* next;
} file;


static inline uint8_t fat16_init(void)
{
	mbr_t *mbr = (mbr_t*) buff;
	vbr_t *vbr = (vbr_t*) buff;
		
	if (mmc_init() != MMC_OK) return 1;
	
  mmc_start_read_block(0);

  // Try sector 0 as a bootsector
	if ((vbr->bsFileSysType[0] == 'F') && (vbr->bsFileSysType[4] == '6'))
	{
		FATRegionStartSec = 0;
	}
	else // Try sector 0 as a MBR	
	{     	 
		FATRegionStartSec = mbr->sector.partition[0].sectorOffset;

		mmc_start_read_block(mbr->sector.partition[0].sectorOffset);
        if ((vbr->bsFileSysType[0] != 'F') || (vbr->bsFileSysType[4] != '6')) {
		      return 2; // No FAT16 found
        }
     }
    
	SectorsPerCluster     = vbr->bsSecPerClus; // 4
	
	// Calculation Algorithms
	FATRegionStartSec    += vbr->bsRsvdSecCnt;						// 6
	RootDirRegionStartSec	= FATRegionStartSec + (vbr->bsNumFATs * vbr->bsNrSeProFAT16);		// 496	
	RootDirRegionSize		 	= (vbr->bsRootEntCnt / 16); 						// 32
	DataRegionStartSec    = RootDirRegionStartSec + RootDirRegionSize;	// 528
	
	return 0;
}

/* ----[ file ]--------------------------------------------------- */

static void inline fat16_readfilesector()
{
	uint16_t clusteroffset;
	uint8_t currentfatsector;
	uint8_t temp, secoffset;
	uint16_t cluster = file.startcluster;
	uint32_t templong;
	
	fatsector_t *fatsector = (fatsector_t*) buff;

	/* SectorsPerCluster is always power of 2 ! */
	secoffset = (uint8_t)file.sector_counter & (SectorsPerCluster-1);
	
	clusteroffset = file.sector_counter;
	temp = SectorsPerCluster >> 1;
	while(temp) {
		clusteroffset >>= 1;
		temp >>= 1;
  }

	currentfatsector = 0xFF;
	while (clusteroffset)
	{
		temp = (uint8_t)((cluster & 0xFF00) >>8);
          
		if (currentfatsector != temp)
		{
			mmc_start_read_block(FATRegionStartSec + temp);

			currentfatsector = temp;
		}
		
		cluster = fatsector->fat_entry[cluster % 256];
		clusteroffset--;
	}

	templong = cluster - 2;
	temp = SectorsPerCluster>>1;
	while(temp) {
		templong <<= 1;	
		temp >>= 1;
	}
		
	/* read the sector of the file into the buffer */
	mmc_start_read_block(templong + DataRegionStartSec + secoffset);
	
	/* advance to next sector */
	file.sector_counter++;
}


static uint8_t file_read_byte() {	// read a byte from the open file from the mmc...
	if (file.next >= buff + 512) {
	    fat16_readfilesector();
		file.next = file.next - 512;
	}
	file.size--;
	return *file.next++;
}

static uint8_t inline gethexnib(char a) {
  a = a & 0x1f;
  if (a >= 0x10)
    return a - 0x10;      // 0-9
  else
    return a - 1 + 0x0a;  // A-F, a-f
}

static uint8_t file_read_hex(void) {
	return (gethexnib(file_read_byte()) << 4) + gethexnib(file_read_byte());
}

static inline void read_hex_file(void) {
	// read current file and convert it from intel hex and flash it, all in the same function
	uint8_t num_flash_words = 0;
	uint8_t* out = pagebuffer;
	flashAddress address = 0;
	while (file.size)
	{
		if (num_flash_words)
		{
			// read (de-hexify)
			*out++ = file_read_hex();
			num_flash_words--;
		
			// if pagebuffer is full
			if (out - pagebuffer == SPM_PAGESIZE) {
        // write page
        write_flash_page(address);
        address += SPM_PAGESIZE;
				out = pagebuffer;
			}
		} 
		else
		{
			// skip bytes until we find another ':'
			if (file_read_byte() == ':') {
				num_flash_words = file_read_hex();  // number of words on line
				file.next+=4;                       // skip 4 bytes of address
#ifdef LARGE_ADDR_SPACE
				uint8_t recordt = file_read_hex();
				if (recordt == 0) continue;
				else if (recordt == 1) break;
				else num_flash_words = 0;
#else
				if (file_read_hex()) break;         // 00 for data, 01 end of file
#endif
			}
		}
	}
	if (out != pagebuffer) write_flash_page(address);
}


/* ----[ directory entry checks ]--------------------------------------------------- */

static inline uint8_t get_eeprom(void *addr) {
#if defined(__AVR_ATmega168__)  || defined(__AVR_ATmega328P__)
		while(EECR & (1<<EEPE));
		EEAR = addr;
		EECR |= (1<<EERE);
		return EEDR;
#else
		return eeprom_read_byte(addr);
#endif
}

static inline uint8_t check_file(direntry_t * dir) {
	uint8_t i;

	/* if file is empty, return */
	if ((dir->fstclust == 0))
		return false;

	/* fill in the file structure */
	file.startcluster = dir->fstclust;
	file.size = dir->filesize;
	file.sector_counter = 0;
	file.next = buff + 512; /* this triggers a new sector load when reading first byte... */

	// compare name to EEPROM 8.3, 11 chars
  for (i=0; i<11; i++) 
		if (get_eeprom(E2END - i) != dir->name[i])
      return false;
    
	// match
  return true;
}


void mmc_updater() {
  //if (get_eeprom(E2END) == 0xff) return;

  if (fat16_init() != 0) return;	

	uint32_t dirsector = RootDirRegionStartSec;

	/* check all files in the Root of the MMC */
	do {
		// read the sector (16 dir entries)
		mmc_start_read_block(dirsector);

		// check each file
		uint8_t i;
    for (i = 0; i < 16; i++) {
			direntry_t* dir = (direntry_t *) buff + i;
			if (check_file(dir)) {
	      read_hex_file();
        return;
      }
		}
		dirsector++;

	} while(--RootDirRegionSize);
}

#endif // MMC_CS
