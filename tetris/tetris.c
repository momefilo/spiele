// momefilo Desing
#include "../include/libs/ili9341/ili9341.h"
#include "../include/melodys.h"
#include "../include/buttons.h"
#include "../include/ranking.h"
#include "graphics/blocks15x15.h"
#include "tetris.h"
#include "hardware/adc.h"
#include <stdlib.h>

//Variablen
struct  _fig Figuren[7], ActFigur;
int Salz;
int MyWaitTime = 5;
int MyHoldTime = 120;
uint32_t Score = 0;
uint16_t Fallspeed = 400; // Fallspeed min 50(fastest) max 500(slowest)
uint8_t LastFigurId = 0;
uint8_t GameArea[16][20];
uint8_t DigitsOffset = 20;
uint8_t Movecounter = 0; // left-, right-, rotate- moves bevor Fall
bool MyButton_pressed = false;
bool IsBeforeLastFigure = false;

//Vorwaertsdeklaration
void clear_GameArea();
void paint_ActFigur();
bool new_ActFigur();

//Implementation
void tetris_init(uint8_t progId){
	buttons_init();
	ili9341_init();
	setOrientation(VERTICAL);
	melodys_init();
	ranking_init(progId);
	clear_GameArea();
	Score = 0;
	paint_Score(Score);
	// Das Salz initialisieren
	adc_init();
	adc_select_input(0);
	Salz = adc_read()*2;
	sleep_us(1);
	adc_select_input(1);
	Salz = Salz + adc_read()+1;
	srand(Salz);
	for(int i=0; i<7; i++){
		Figuren[i].id = i;
		Figuren[i].pos.x = 7;
		Figuren[i].pos.y = 0;
		Figuren[i].rotateCnt = 0;
	}
	Figuren[0].row[0] = 0x0110;
	Figuren[0].row[1] = 0x0110;
	Figuren[0].row[2] = 0x0000;
	Figuren[0].row[3] = 0x0000;
	Figuren[0].lzero[0]=1;
	Figuren[0].rzero[0]=1;
	Figuren[0].lzero[1]=1;
	Figuren[0].rzero[1]=1;
	Figuren[0].lzero[2]=0;
	Figuren[0].rzero[2]=2;
	Figuren[0].lzero[3]=0;
	Figuren[0].rzero[3]=2;
	Figuren[0].bzero[0]=2;
	Figuren[0].bzero[1]=1;
	Figuren[0].bzero[2]=1;
	Figuren[0].bzero[3]=2;

	Figuren[1].row[0] = 0x0200;
	Figuren[1].row[1] = 0x2220;
	Figuren[1].row[2] = 0x0000;
	Figuren[1].row[3] = 0x0000;
	Figuren[1].lzero[0]=0;
	Figuren[1].rzero[0]=1;
	Figuren[1].lzero[1]=1;
	Figuren[1].rzero[1]=1;
	Figuren[1].lzero[2]=0;
	Figuren[1].rzero[2]=1;
	Figuren[1].lzero[3]=0;
	Figuren[1].rzero[3]=2;
	Figuren[1].bzero[0]=2;
	Figuren[1].bzero[1]=1;
	Figuren[1].bzero[2]=1;
	Figuren[1].bzero[3]=1;

	Figuren[2].row[0] = 0x0300;
	Figuren[2].row[1] = 0x0300;
	Figuren[2].row[2] = 0x0300;
	Figuren[2].row[3] = 0x0300;
	Figuren[2].lzero[0]=1;
	Figuren[2].rzero[0]=2;
	Figuren[2].lzero[1]=0;
	Figuren[2].rzero[1]=0;
	Figuren[2].lzero[2]=1;
	Figuren[2].rzero[2]=2;
	Figuren[2].lzero[3]=0;
	Figuren[2].rzero[3]=0;
	Figuren[2].bzero[0]=0;
	Figuren[2].bzero[1]=2;
	Figuren[2].bzero[2]=0;
	Figuren[2].bzero[3]=2;

	Figuren[3].row[0] = 0x0400;
	Figuren[3].row[1] = 0x0440;
	Figuren[3].row[2] = 0x0040;
	Figuren[3].row[3] = 0x0000;
	Figuren[3].lzero[0]=1;
	Figuren[3].rzero[0]=1;
	Figuren[3].lzero[1]=0;
	Figuren[3].rzero[1]=1;
	Figuren[3].lzero[2]=0;
	Figuren[3].rzero[2]=2;
	Figuren[3].lzero[3]=0;
	Figuren[3].rzero[3]=1;
	Figuren[3].bzero[0]=1;
	Figuren[3].bzero[1]=1;
	Figuren[3].bzero[2]=1;
	Figuren[3].bzero[3]=2;

	Figuren[4].row[0] = 0x0050;
	Figuren[4].row[1] = 0x0550;
	Figuren[4].row[2] = 0x0500;
	Figuren[4].row[3] = 0x0000;
	Figuren[4].lzero[0]=1;
	Figuren[4].rzero[0]=1;
	Figuren[4].lzero[1]=0;
	Figuren[4].rzero[1]=1;
	Figuren[4].lzero[2]=0;
	Figuren[4].rzero[2]=2;
	Figuren[4].lzero[3]=0;
	Figuren[4].rzero[3]=1;
	Figuren[4].bzero[0]=1;
	Figuren[4].bzero[1]=1;
	Figuren[4].bzero[2]=1;
	Figuren[4].bzero[3]=2;

	Figuren[5].row[0] = 0x0660;
	Figuren[5].row[1] = 0x0600;
	Figuren[5].row[2] = 0x0600;
	Figuren[5].row[3] = 0x0000;
	Figuren[5].lzero[0]=1;
	Figuren[5].rzero[0]=1;
	Figuren[5].lzero[1]=0;
	Figuren[5].rzero[1]=1;
	Figuren[5].lzero[2]=0;
	Figuren[5].rzero[2]=2;
	Figuren[5].lzero[3]=0;
	Figuren[5].rzero[3]=1;
	Figuren[5].bzero[0]=1;
	Figuren[5].bzero[1]=1;
	Figuren[5].bzero[2]=1;
	Figuren[5].bzero[3]=2;

	Figuren[6].row[0] = 0x0770;
	Figuren[6].row[1] = 0x0070;
	Figuren[6].row[2] = 0x0070;
	Figuren[6].row[3] = 0x0000;
	Figuren[6].lzero[0]=1;
	Figuren[6].rzero[0]=1;
	Figuren[6].lzero[1]=0;
	Figuren[6].rzero[1]=1;
	Figuren[6].lzero[2]=0;
	Figuren[6].rzero[2]=2;
	Figuren[6].lzero[3]=0;
	Figuren[6].rzero[3]=1;
	Figuren[6].bzero[0]=1;
	Figuren[6].bzero[1]=1;
	Figuren[6].bzero[2]=1;
	Figuren[6].bzero[3]=2;
	new_ActFigur();
}

void reset_Movecounter(){ Movecounter = 0; }

uint16_t get_Fallspeed(){ return Fallspeed; }

/* GameArea mit Nullen fuellen und Display leeren */
void clear_GameArea(){
	for(int y=0; y<20; y++){
		for(int x=0; x<16; x++){
			uint16_t pos[] = {
				x * 15,
				y * 15 + DigitsOffset,
				x * 15 + 14,
				y * 15 + 14 + DigitsOffset
			};
			drawRect(pos, BLOCK[0]);
			GameArea[x][y] = 0;
		}
	}
}

/* Zeile animieren check_Rows-Hilfsfunktion */
void animate_Row(uint8_t zeile){
	/*die Zeile von der mitte aus nach links und rechts gleichzeitig
	 * mit einer Farbe gefüllt, und dies nacheinander mit allen allen Farben
	 */
	for(int color=7; color >= 0; color--){
		for(int x=0; x<8; x++){
			uint16_t lpos[]={
				(7 - x) * 15,
				zeile * 15 + DigitsOffset,
				(7 - x) * 15 + 14,
				zeile * 15 + DigitsOffset + 14
			};
			uint16_t rpos[]={
				(8 + x) * 15,
				zeile * 15 + DigitsOffset,
				(8 + x) * 15 + 14,
				zeile * 15 + DigitsOffset + 14
			};
			drawRect(lpos, BLOCK[color]);
			drawRect(rpos, BLOCK[color]);
			sleep_ms(5);
		}
		sleep_ms(5);
	}
	melodys_play(SOUND_ROW);
}

/* GameArea komplett neu zeichnen check_Rows-Hilfsfunktion */
void paint_GameArea(){
	for(int y=0; y<20; y++){
		for(int x=0; x<16; x++){
			uint16_t pos[] = {
				x * 15,
				y * 15 + DigitsOffset,
				x * 15 + 14,
				y * 15 + DigitsOffset + 14};
			drawRect(pos, BLOCK[GameArea[x][y]]);
		}
	}
}

/* pruefen wieviele Zeilen vollständig sind und mit Animation loeschen */
uint8_t check_Rows(){
	uint8_t rowCount = 0; //Anzahl kompletter Zeilen

	//die vier Zeilen der Figur durchlaufen
	for(int y=ActFigur.pos.y; y<ActFigur.pos.y+4; y++){
		if(y<20){ // GameAreagrenze nicht überschreiten
			bool full = true;

			//pruefen ob ein Feld 0 hat und dann full auf false setzen
			for(int x=0; x<16; x++){ if(GameArea[x][y] < 1) full = false;}

			//wenn Zeile vollstaendig
			if(full){
				rowCount++;
				animate_Row(y);
				//darueber liegende Zeilen nach unten ruecken
				for(int y2=y; y2>0; y2--){
					for(int x2=0; x2<16; x2++){
						GameArea[x2][y2] = GameArea[x2][y2 - 1];
					}
				}
				paint_GameArea();
			}
		}
	}

	return rowCount;
}

/* Gibt true zurueckWenn die Position für die 4x4 Matrix im GameArea frei ist */
bool check_Position(uint16_t *rows, struct POS mypos){
	for(int i=0; i<4; i++){
		uint16_t ypos = mypos.y + i;
		uint16_t bitmask = 0xF000;
		for(int k=0; k<4; k++){
			uint16_t xpos = mypos.x - 1 + k;
			if(xpos >= 0 && xpos < 16 && ypos < 20){
				if((rows[i] & bitmask) && GameArea[xpos][ypos] ){
						return false;
				}
			}
			bitmask = (bitmask >> 4);
		}
	}
	return true;
}

/* neue Figur mit Positionscheck fuer gameover */
bool new_ActFigur(){
	bool randIsOk = false;
	uint8_t newId;
	while(! randIsOk){
		int r = rand();
		newId = (r/1)%7;
		if(newId == LastFigurId){
			if(! IsBeforeLastFigure){
				IsBeforeLastFigure = true;
				randIsOk = true;
			}
		}
		else{
			IsBeforeLastFigure = false;
			LastFigurId = newId;
			randIsOk = true;
		}
	}
	ActFigur = Figuren[newId];
	paint_ActFigur();

	return (check_Position(ActFigur.row, ActFigur.pos));
}

/* Speichert die aktuelle Figur im GameArea (wenn diese nicht mehr fallen kann) */
void set_ActFigur(){
	for(int i=0; i<4; i++){
		uint16_t ypos = ActFigur.pos.y + i;
		uint16_t bitmask = 0xF000;
		for(int k=0; k<4; k++){
			uint16_t xpos = ActFigur.pos.x - 1 + k;
			uint8_t color = (ActFigur.row[i] & bitmask) >> (12 - 4*k);
			if(xpos >= 0 && xpos < 16 && ypos < 20 && color > 0){
				GameArea[xpos][ypos] = color;
			}
			bitmask = (bitmask >> 4);
		}
	}
}

/* Rotiert die Figur */
bool rotate_ActFigur(){
	//Figur 0 (Wuerfel) wird nicht gedreht
	if(ActFigur.id == 0) return false;
	//pruefen ob die Roation der Figur diese ueber den Rand hinaus dreht
	uint8_t my_rotate_cnt = ActFigur.rotateCnt + 1;
	if(my_rotate_cnt > 3)my_rotate_cnt = 0;
	uint8_t loffset = ActFigur.lzero[my_rotate_cnt];
	uint8_t roffset = ActFigur.rzero[my_rotate_cnt];
	uint8_t boffset = ActFigur.bzero[my_rotate_cnt];
	if( ((ActFigur.pos.x + loffset) < 1) || \
		((ActFigur.pos.x - roffset) > 13) || \
		((ActFigur.pos.y - boffset) > 16)) return false;

	uint16_t tmp0 = ActFigur.row[0];
	uint16_t tmp1 = ActFigur.row[1];
	uint16_t tmp2 = ActFigur.row[2];
	uint16_t tmp3 = ActFigur.row[3];
	uint16_t check[4];
	check[0] = (0xF000 & tmp2) | \
					((0xF000 & tmp1)>>4) | \
					((0xF000 & tmp0) >>8);
	check[1] = ((0x0F00 & tmp2)<<4) | \
					(0x0F00 & tmp1) | \
					((0x0F00 & tmp0)>>4) | \
					((0x0F00 & tmp3)>>8);
	check[2] = ((0x00F0 & tmp2)<<8) | \
					((0x00F0 & tmp1)<<4) | \
					(0x00F0 & tmp0);
	check[3] = ((0x000F & tmp1)<<8);
	if(check_Position(check, ActFigur.pos)){
		for(int i=0; i<4; i++){
			uint16_t bitmask = 0xF000;
			for(int k=0; k<4; k++){
				uint16_t pos[] = {
					(ActFigur.pos.x - 1 + k) * 15,
					(ActFigur.pos.y + i) * 15 + DigitsOffset,
					(ActFigur.pos.x - 1 + k) * 15 + 14,
					(ActFigur.pos.y + i) * 15 + DigitsOffset + 14
				};
				uint8_t empty = (ActFigur.row[i] & bitmask) >> (12 - 4*k);
				uint8_t color = (check[i] & bitmask) >> (12 - 4*k);
				if(empty > 0 && color < 1){
					drawRect(pos, BLOCK[0]);
				}
				if(color > 0){
					drawRect(pos, BLOCK[color]);
				}
				bitmask = (bitmask >> 4);
			}
			ActFigur.row[i] = check[i];
		}
		ActFigur.rotateCnt++;
		if(ActFigur.rotateCnt>3)ActFigur.rotateCnt = 0;
		return true;
	}
	else return false;
}

/* Die Figur faellt */
bool fall_ActFigur(){
	//Figur hat boden erreicht?
	if((ActFigur.pos.y - ActFigur.bzero[ActFigur.rotateCnt]) > 15)return false;

	//prüfen ob die neue Position frei ist
	struct POS movepos = ActFigur.pos;
	movepos.y++;
	if(check_Position(ActFigur.row, movepos)){

		//verschieben
		for(int i=3; i>=0; i--){
			uint16_t bitmask = 0xF000;
			for(int k=0; k<4; k++){
				movepos.x = ((ActFigur.pos.x - 1 + k) * 15);
				uint16_t pos[] = { movepos.x, movepos.y+ DigitsOffset,
							 movepos.x + 14, movepos.y + DigitsOffset + 14 };
				uint8_t color = (ActFigur.row[i] & bitmask) >> (12 - 4*k);
				if(color > 0){ // nur nichtleere Felder verschieben
					for(int m=1; m<16; m++){
						movepos.y = ((ActFigur.pos.y  + i)* 15) + m;
						pos[1] = movepos.y + DigitsOffset; pos[3] = movepos.y + DigitsOffset + 14;
						drawRect(pos, BLOCK[color]);
					}
					//altes feld leeren
					movepos.y = ((ActFigur.pos.y  + i)* 15);
					pos[1] = movepos.y + DigitsOffset;
					pos[3] = movepos.y + 14 + DigitsOffset;
					drawRect(pos, BLOCK[0]);
				}
				bitmask = (bitmask >> 4);
			}

		}
		ActFigur.pos.y++;
		return true;
	}
	else return false;
}

/* Verschiebt die Figur nach links oder rechts */
bool move_ActFigur(int direction){
	if(direction == DIR_LEFT){
		// Prüfen ob noch Raum zum Verschieben nach links vorhanden ist
		uint8_t leftside = ActFigur.pos.x - 1 + ActFigur.lzero[ActFigur.rotateCnt];
		if(leftside < 1) return false;

		//prüfen ob die neue Position frei ist
		struct POS movepos = ActFigur.pos;
		movepos.x--;
		if(check_Position(ActFigur.row, movepos)){

			//verschieben
			for(int i=0; i<4; i++){
				movepos.y = ((ActFigur.pos.y  + i)* 15);
				uint16_t pos[] = { movepos.x, movepos.y+ DigitsOffset,
								movepos.x + 14, movepos.y + DigitsOffset + 14 };
				uint16_t bitmask = 0xF000;
				for(int k=0; k<4; k++){
					uint8_t color = (ActFigur.row[i] & bitmask) >> (12 - 4*k);
					if(color > 0){ // nur nichtleere Felder verschieben
						for(int m=1; m<16; m++){
							movepos.x = ((ActFigur.pos.x - 1 + k) * 15) - m;
							pos[0] = movepos.x; pos[2] = movepos.x + 14;
							drawRect(pos, BLOCK[color]);
						}
						//altes feld leeren
						movepos.x = (ActFigur.pos.x - 1 + k) * 15;
						pos[0] = movepos.x;
						pos[2] = movepos.x + 14;
						drawRect(pos, BLOCK[0]);
					}
					bitmask = (bitmask >> 4);
				}

			}
			ActFigur.pos.x--;
			return true;
		}
		else return false;
	}
	if(direction == DIR_RIGHT){
		// Prüfen ob noch Raum zum Verschieben nach links vorhanden ist
		uint8_t rightside = ActFigur.pos.x + 2 - ActFigur.rzero[ActFigur.rotateCnt];
		if(rightside > 14) return false;

		//prüfen ob die neue Position frei ist
		struct POS movepos = ActFigur.pos;
		movepos.x++;
		if(check_Position(ActFigur.row, movepos)){

			//verschieben
			for(int i=0; i<4; i++){
				movepos.y = (ActFigur.pos.y  + i) * 15;
				uint16_t pos[] = { movepos.x, movepos.y+ DigitsOffset,
								movepos.x + 14, movepos.y + DigitsOffset + 14 };
				uint16_t bitmask = 0x000F;
				for(int k=0; k<4; k++){
					uint8_t color = (ActFigur.row[i] & bitmask) >> (4*k);
					if(color > 0){
						for(int m=1; m<16; m++){
							movepos.x = ((ActFigur.pos.x + 2 - k) * 15) + m;
							pos[0] = movepos.x; pos[2] = movepos.x + 14;
							drawRect( pos, BLOCK[color]);
						}
						//altes Feld leeren
						movepos.x = (ActFigur.pos.x + 2 - k) * 15;
						pos[0] = movepos.x; pos[2] = movepos.x + 14;
						drawRect(pos, BLOCK[0]);
					}
					bitmask = (bitmask << 4);
				}

			}
			ActFigur.pos.x++;
			return true;
		}
		else return false;
	}
}

/* Zeichnet die Figur aufs Display*/
void paint_ActFigur(){
	for(int i=0; i<4; i++){
		uint16_t bitmask = 0xF000;
		for(int k=0; k<4; k++){
			uint8_t color = (ActFigur.row[i] & bitmask) >> (12 - 4*k);
			if(color > 0){
				uint16_t pos[] = {
					(ActFigur.pos.x - 1 + k) * 15,
					(ActFigur.pos.y + i) * 15 + DigitsOffset,
					(ActFigur.pos.x - 1 + k) * 15 + 14,
					(ActFigur.pos.y + i) * 15 + DigitsOffset + 14
				};
				drawRect(pos, BLOCK[color]);
			}
			bitmask = (bitmask >> 4);
		}
	}
}

bool new_Game(){
	bool ret = false;
	Fallspeed = 400;
	Movecounter = 0;
	uint8_t ranking = set_Score(Score);
	paint_Highscore();
	if(ranking > 0){
		paint_Rang(ranking);
		if(ranking < 2)melodys_play(SOUND_YEH);
		else melodys_play(SOUND_YUP);
	}
	else{
		paint_Score(Score);
		melodys_play(SOUND_BAH);
	}
	while( (! gpio_get(BUTTON_R)) && (! gpio_get(BUTTON_L) && \
		(! gpio_get(BUTTON_U)) && (! gpio_get(BUTTON_D))) ){}
	paint_Menu();
	sleep_ms(500);
	bool auswahl = false;
	MyButton_pressed = false;
	while(! auswahl){
		if(gpio_get(BUTTON_U) || gpio_get(BUTTON_D)){
			if(MyButton_pressed){
				if(gpio_get(BUTTON_U)){
					ret = true; auswahl = true;
					MyButton_pressed = false;
					sleep_ms(MyHoldTime);
				}
				if(gpio_get(BUTTON_D)){
					ret = false; auswahl = true;
					MyButton_pressed = false;
					sleep_ms(MyHoldTime);
				}
			}
			else{
				MyButton_pressed = true;
				sleep_ms(MyWaitTime);
			}
		}
	}
	Score = 0;
	paint_Score(Score);
	clear_GameArea();
	return ret;
}

void inkrase_Fallspeed(){
	if(Fallspeed < 100) Fallspeed--;
	else if(Fallspeed < 360){ Fallspeed = Fallspeed - 5; }
	else {Fallspeed = Fallspeed - 10; }
}

bool end_fall(bool newGame){
	melodys_play(SOUND_FALL);
	set_ActFigur(); // speichern der position im Gamearea
	uint32_t points = check_Rows(); // pruefen wieviele Zeilen komplett sind
	if(points > 0){
		inkrase_Fallspeed();
		Score = Score + 100;
		if(points > 1){
			Score = Score + 200;
			if(points > 2){
				Score = Score + 300;
				if(points > 3) Score = Score + 400;
			}
		}
		paint_Score(Score); // neuen Score anzeigen
	}
	// neue Figur setzten, wenn kein platz, neues spiel
	if(! new_ActFigur()){ if(newGame) return new_Game(); }
	return true;
}

uint8_t get_Direction(){ // prueft ob eine Taste gedrueckt ist und ruft Funktionen auf
	if(MyButton_pressed){
		if(gpio_get(BUTTON_U)){
			rotate_ActFigur();
			sleep_ms(MyHoldTime*3/2);
			MyButton_pressed = false;
			Movecounter++;
		}else
		if(gpio_get(BUTTON_D)){
			if(! fall_ActFigur()) end_fall(false);
			sleep_ms(MyHoldTime/2);
			MyButton_pressed = false;
			Movecounter++;
		}else
		if(gpio_get(BUTTON_R)){
			move_ActFigur(DIR_RIGHT);
			sleep_ms(MyHoldTime*4/3);
			MyButton_pressed = false;
			Movecounter++;
		}else
		if(gpio_get(BUTTON_L)){
			move_ActFigur(DIR_LEFT);
			sleep_ms(MyHoldTime*4/3);
			MyButton_pressed = false;
			Movecounter++;
		}
	}
	else{
		MyButton_pressed = true;
		sleep_ms(MyWaitTime);
	}
	return Movecounter;
}
