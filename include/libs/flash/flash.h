// momefilo Desing
#ifndef momefilo_flash_h
#define momefilo_myflash_h 1

#include "pico/stdlib.h"

/* Initialisiert den mit stage bezeichneten 4096 Byte grossen Sektor zu einem uint32_t Arry mit
 * der Groesse 63
 * stage >=0; die Id des Sektors welcher benutzt wird*/
void flash_init(uint8_t stage);

void flash_salzInit();
int flash_salzPlus();

/* Schreibt data an Position id im uint32_t Array
 * id >= 0 && id <63: Position im uint32_t Array
 * data = uint32_t; Daten die in uint32_t Array an Position id gespeichert werden*/
void flash_setData(uint8_t id, uint32_t data);

/* Schreibt fortlaufend daten von Position start bis end im uint32_t Array aus *data
 * start >= 0 && start < 62: Startposition im uint32_t Array
 * end > 0 && end < 63 Endposition im uint32_t Array */
void flash_setDataRow(uint8_t start, uint8_t end, uint32_t *data);

/* Gibt das Daten-Array aus 63 uint32_t Werten zurueck*/
uint32_t* flash_getData();

#endif
