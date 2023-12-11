// momefilo Desing
#ifndef momefilo_buttons_h
#define momefilo_buttons_h 1

#include "pico/stdlib.h"

#define BUTTON_U 6
#define BUTTON_D 7
#define BUTTON_R 8
#define BUTTON_L 9

/* Initialisiert gpio */
void buttons_init();

/* Gibt den aktuell gedrueckten Button zurück oder den Wert 100 */
uint8_t get_Button();
#endif
