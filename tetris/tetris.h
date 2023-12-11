// momefilo Desing
#ifndef tetris_h
#define tetris_h 1

#include "pico/stdlib.h"

#define DIR_LEFT  10
#define DIR_RIGHT  11

struct POS{
	uint16_t x;
	uint16_t y;
};

/* Die Figuren des Spiels mit Kommentaren */
struct _fig{
	uint8_t id;

	/* Jede Figur besteht aus einer 4x4 Matrix
	 * eine Zeile hat vier hexadezimale Stellen deren Wert die Farbe jenes Feldes
	 * der Figur festlegt, bei Werten groesser 0. Felder mt 0 bleiben leer.
	 */
	uint16_t row[4];

	/* Die Position des zweiten Feldes von rechts-oben im Gamearea
	 */
	struct POS pos;

	/* Maximal vier Rotationen im Uhrzeigersinn die hier gezaehlt werden. Die Felder
	 * l-, r-, b-zero werden antsprechend dieser Variable ausgewertet um die Grenzen
	 * der Figur im Gamearea zu ermitteln
	 */
	uint8_t rotateCnt;

	/* Die Anzahl der aeusserst linken Spalten deren vier Zeilen alle 0 sind,
	 * wird mittels rotateCnt entsprechend der nächsten Rotationsposition hier
	 * heraus gelesen; um ein Rotieren der Figur ueber den Rand hinaus zu erkennen!
	 */
	uint8_t lzero[4];

	/* Die Anzahl der aeusserst rechten Spalten deren vier Zeilen alle 0 sind
	 */
	uint8_t rzero[4];

	/* Die Anzahl der untersten Zeilen deren vier Spalten alle 0 sind
	 */
	 uint8_t bzero[4];
};

/* Diese Tertris hat die klassischen sieben Figuren die aus einer 4x4 Matrix bestehen
 * und sich in einer 16x20 Spalten Matrix bewegen. Diese Funktion erstellt diese
 * Figuren. Siehe Komentierung des "struc _fig" in figuren.h für Details
 * Das Salz wird fuer die Zufallszahl der naechsten Figur mit den ADC 1 und 2
 * initialisiert
 */
void tetris_init(uint8_t progId);

/* Movecounter wird zum Ruecksprung aus der Schleife benoetigt */
void reset_Movecounter();

/* Die Figur faellt */
bool fall_ActFigur();

/* ist ein Button gedrueckt */
uint8_t get_Direction();

/* die Fallgeschwindigkeit */
uint16_t get_Fallspeed();

/* Die Figur kann nicht mehr fallen. Der Parameter newGame wird benoetigt um zu kennzeichnen welche
 * Funktion die Aufrufende ist. Die Hauptspielschleife in "spiele.c" darf bei gedruecktem taster
 * nicht den returnwert aus der Spielschleife erhalten*/
bool end_fall(bool newGame);
#endif
