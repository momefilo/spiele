// momefilo Desing
#ifndef buttons_h
#define buttons_h 1

#define BUTTON_U 6
#define BUTTON_D 7
#define BUTTON_L 8
#define BUTTON_R 9

/* Initialisiert gpio */
void buttons_init();

/* Gibt den aktuell gedrueckten Button zurück oder den Wert 100 */
uint8_t get_Button();
#endif
