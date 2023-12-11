// momefilo Desing
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "flash.h"

#include <stdio.h>
int Salt;
uint32_t Flash_Offset, Salz_Offset;
uint8_t *Flash_Content, *Salz_Content, Salz_PageWerte[256];
uint8_t Pagecount, SectorOffset = 34, Salz_Page, Salz_Byte;
uint32_t Data[63];

int flash_getSalz(){ return Salt; }
void set_Salt(){
	uint8_t salz = 0, wert = Salz_PageWerte[Salz_Byte];
	if(wert > 0x7E) salz = 1;
	else if(wert > 0x3E) salz = 2;
	else if(wert > 0x1E) salz = 3;
	else if(wert > 0x0E) salz = 4;
	else if(wert > 0x06) salz = 5;
	else if(wert > 0x02) salz = 6;
	else if(wert > 0x00) salz = 7;
	else salz = 8;
	Salt = Salz_Page * 256*8 + Salz_Byte*8 + salz;
}
void flash_salzInit(){
	Salz_Offset = (PICO_FLASH_SIZE_BYTES - SectorOffset * FLASH_SECTOR_SIZE);
	Salz_Content = (uint8_t *) (XIP_BASE + Salz_Offset);
	Salz_Page = 0;
	Salz_Byte = 0;
	bool found = false;
	for(uint8_t myx=0; myx<16; myx++){
		for(uint16_t myy=0; myy<256; myy++){
			if(Salz_Content[(15-myx)*256 + 255-myy] != 0xFF){
				for(uint16_t y=0; y<256; y++){
					if(y<255-myy)Salz_PageWerte[y] = 0;
					else if(y==255-myy)Salz_PageWerte[y] = Salz_Content[(15-myx)*256 + 255-myy];
					else if(y>255-myy)Salz_PageWerte[y] = 0xFF;
				}
				Salz_Page = 15 - myx;
				Salz_Byte = 255 - myy;
				found = true;
				break;
			}
		}
		if(found) break;
	}
	if(! found) for(int i=0; i<256; i++)Salz_PageWerte[i]=0xFF;
	set_Salt();
}
int flash_salzPlus(){
	uint32_t flags = save_and_disable_interrupts();
	if(Salz_PageWerte[Salz_Byte]>0) Salz_PageWerte[Salz_Byte] = Salz_PageWerte[Salz_Byte] / 2;
	else if(Salz_Byte < 255){
		Salz_Byte++;
		Salz_PageWerte[Salz_Byte] = Salz_PageWerte[Salz_Byte] / 2;
	}
	else if(Salz_Page < 15){
		Salz_Page++;
		Salz_Byte = 0;
		for(uint16_t y=0; y<256; y++) Salz_PageWerte[y] = 0xFF;
		Salz_PageWerte[Salz_Byte] = Salz_PageWerte[Salz_Byte] / 2;
	}
	else{
		flash_range_erase(Salz_Offset, FLASH_SECTOR_SIZE);
		Salz_Page = 0;
		Salz_Byte = 0;
		for(uint16_t y=0; y<256; y++) Salz_PageWerte[y] = 0xFF;
		Salz_PageWerte[Salz_Byte] = Salz_PageWerte[Salz_Byte] / 2;
	}
	flash_range_program(Salz_Offset + Salz_Page * FLASH_PAGE_SIZE, Salz_PageWerte, FLASH_PAGE_SIZE);
	restore_interrupts(flags);
	set_Salt();
	return Salt;
}

/* Pueft ob die letzte der 16 256Byte-Pages erreicht ist und loescht in diesem Falle den gesamten
 * 4096Byte-Sektor bevor die neue Page geschrieben wird*/
void write_Flash(){
	uint32_t flags = save_and_disable_interrupts();
	uint8_t buf[FLASH_PAGE_SIZE];
	if(Pagecount > 15){
		flash_range_erase(Flash_Offset, FLASH_SECTOR_SIZE);
		Pagecount = 0;
	}
	buf[0] = 0xAB;
	buf[1] = 0xBA;
	uint8_t bufcount = 0;
	for(int i=0; i<63; i++){
		uint32_t mask = 0xFF000000;
		for(int k=0; k<4; k++){
			buf[2 + bufcount] = (Data[i] & mask) >> (24 - 8*k);
			mask = mask >> 8;
			bufcount++;
		}
	}
	flash_range_program(Flash_Offset + Pagecount * FLASH_PAGE_SIZE, buf, FLASH_PAGE_SIZE);
	restore_interrupts(flags);
	Pagecount++;
}

/* Liest Data aus der aktuellen 256byte-Page */
void get_Flash(){
	int addr;
	if(Pagecount < 1){
		addr = 15 * 256;
	}else{ addr = (Pagecount - 1) * 256;}
	uint8_t bufcount = 0;
	for(int i=0; i<63; i++){
		for(int k=0; k<4; k++){
			Data[i] |= (Flash_Content[addr + 2 + bufcount]) << (24 - 8*k);
			bufcount++;
		}
	}
}

/* Prueft welcher 256byte Block aktuell ist und ruft get_Flash auf. Ist kein Speicher
 * mit dem Merkmal 0xAB an erster, und 0xBA an zweiter Stelle auffindbar wird
 * Pagecount auf 16 gesetzt so das writeFlash den Speicher mit 4096byte neu anlegt
 * stage gibt den zu lesen/schreibenden Sector an,
 * es können mehrere (von oben herab) verwaltet werden */
void flash_init(uint8_t stage){
	Flash_Offset = (PICO_FLASH_SIZE_BYTES - (stage + 1 + SectorOffset) * FLASH_SECTOR_SIZE);
	Flash_Content = (uint8_t *) (XIP_BASE + Flash_Offset);
//	flash_range_erase(PICO_FLASH_SIZE_BYTES - 64*FLASH_SECTOR_SIZE, 64*FLASH_SECTOR_SIZE);
	for(uint8_t i=0; i<63; i++){ Data[i] = 0; }
	bool found = false;
	Pagecount = 15;

	for(int pc=Pagecount; pc>=0; pc--){
			uint16_t addr = pc * 256;
			if( (Flash_Content[addr] == 0xAB) && \
							(Flash_Content[addr+1] == 0xBA)){
				found = true;
				Pagecount = pc + 1;
				get_Flash();
				return;

		}
	}
	if(! found) Pagecount = 16;
}

/* Gibt das Daten-Array aus 63 uint32_t Werten zurueck*/
uint32_t *flash_getData(){
	return Data;
}

void flash_setData(uint8_t id, uint32_t data){
	Data[id] = data;
	write_Flash();
}

void flash_setDataRow(uint8_t start, uint8_t end, uint32_t *data){
	for(uint8_t	i=start; i<=end; i++){
		 Data[i] = data[i - start];
	}
	write_Flash();
}
