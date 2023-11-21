// momefilo Desing
#include "../libs/flash/flash.h"
#include "../libs/ili9341/ili9341.h"
#include "digits/digit15x20.h"
#include "digits/pigit15x20.h"
#include "digits/rang25x25.h"
#include "../ranking.h"

uint32_t Highscore[HIGHSCORECOUNT];


/* Prueft ob der score ein Highscore ist und gibt die Position dessen zurueck und
 * speicher den Highscore im Flash
 * ist der score nicht im Highscore wird 0 zurueckgegeben und nicht gespeichert
 */
uint8_t set_Score(uint32_t score){
	for(int i=HIGHSCORECOUNT - 1; i>=0; i--){
		if(score == Highscore[i]){ return HIGHSCORECOUNT - i;}
		else if(score > Highscore[i]){
			for(int k=0; k<i; k++) Highscore[k] = Highscore[k + 1];
			Highscore[i] = score;
			flash_setDataRow(0, (HIGHSCORECOUNT - 1), Highscore);
			return HIGHSCORECOUNT - i;
		}
	}
	return 0;
}

/* Schreibt die Ziffern einer sechsstelligen Zahl ins uebergebene digits-Array*/
void get_Digits(uint32_t zahl, uint8_t *digits){
	digits[0] = zahl % 10;
	digits[1] = (zahl % 100 - zahl % 10) / 10;
	digits[2] = (zahl % 1000 - zahl % 100) / 100;
	digits[3] = (zahl % 10000 - zahl % 1000) / 1000;
	digits[4] = (zahl % 100000 - zahl % 10000) / 10000;
	digits[5] = (zahl % 1000000 - zahl % 100000) / 100000;
}

/* Leert die Scorezeile (erste 20Pixel hohe Zeile) im Display */
void clear_Score(){
	for(int i=0; i<16; i++){
		uint16_t pos[] = {
			i * 15,
			0,
			i * 15 + 14,
			19
		};
		drawRect(pos, DIGIT[10]);
	}
}

void ranking_init(uint8_t progId){
	flash_init(progId);
	uint32_t *highscore = flash_getData();
	for(uint8_t i=0; i<HIGHSCORECOUNT; i++)Highscore[i] = highscore[i];

}

//not so noce
void paint_Menu(){
	//x0=41;y0=36;x1=200 ;y1=233
	uint16_t area[] = {41,36,51,233};
	paintRectGradient(area, 0xFFF0, 0x001F);
	area[0] = 52; area[2] = 190;
	paintRect(area, 0x841F);
	area[0] = 191; area[2] = 200;
	paintRectGradient(area, 0x001F, 0xFFF0);
	setBgColor(0x841F);
	setFgColor(0x07E0);
	uint16_t tpos[] = {66,125};
	writeText16x16(tpos, "nochmal", 7, false, false);
	//tpos[0] = 72;
	tpos[1] = 160;
	setFgColor(0xFFE0);
	writeText16x16(tpos, "zurueck", 7, false, false);
}

//not so nice
void paint_Rang(uint32_t rang){
	uint16_t pos[] = { 75, 42, 75+6*15, 72};
	paintRect(pos, 0xF800);
	pos[2]= 89;
	paintRectGradient(pos, 0xFFF0, 0xF800);
	pos[0] = 75+5*15; pos[2] = 75+6*15;
	paintRectGradient(pos, 0xF800, 0xFFF0);
	pos[0] = 108; pos[1] = 45; pos[2] = 132; pos[3] = 69;
	drawRect(pos, RANG[rang - 1]);
}

/* Zeigt den akt. Score links und den naechsthoeheren Highscore rechts  in der
 * Scorzeile an */
void paint_Score(uint32_t score){
	clear_Score();
	uint8_t digits[6];
	get_Digits(score,digits);
	//einer
	uint16_t pos[] = {15 * 15, 0, 15 * 15 + 14, 19};
	drawRect(pos, DIGIT[digits[0]]);
	//zehner
	if(digits[1]>0 || digits[2]>0 || digits[3]>0 || digits[4]>0 || digits[5]>0){
		pos[0] = 14 * 15; pos[2] = 14 * 15 + 14;
		drawRect(pos, DIGIT[digits[1]]);}
	//hundeter
	if(digits[2]>0 || digits[3]>0 || digits[4]>0 || digits[5]>0){
		pos[0] = 13 * 15; pos[2] = 13 * 15 + 14;
		drawRect(pos, DIGIT[digits[2]]);}
	//tausnder
	if(digits[3]>0 || digits[4]>0 || digits[5]>0){
		pos[0] = 12 * 15; pos[2] = 12 * 15 + 14;
		drawRect(pos, DIGIT[digits[3]]);}
	//zehntausnder
	if(digits[4]>0 || digits[5]>0){
		pos[0] = 11 * 15; pos[2] = 11 * 15 + 14;
		drawRect(pos, DIGIT[digits[4]]);}
	//hunderttausnder
	if(digits[5]>0){
		pos[0] = 10 * 15; pos[2] = 10 * 15 + 14;
		drawRect(pos, DIGIT[digits[5]]);}
	//paint the next greater Highscore on the left side on digitsline in display
	bool newTopScore = true;
	for(int i=0; i<HIGHSCORECOUNT; i++){
		if(Highscore[i] > score){
			get_Digits(Highscore[i],digits);
			newTopScore = false;
			//ermitteln der aeusserst rechten Position des Highscore
			int positionOffset = 0;
			for(int k=5; k>0; k--){
				if(digits[k] > 0) break;
				positionOffset++;
			}
			//wenn der rang des Highscore zweistellig ist (10)
			//nicht benutzt da nur fuenf Raenge
			if(5 < 1){
				positionOffset--;
//				Display_paint_Score(PIGIT[1], 0);
//				Display_paint_Score(PIGIT[0], 1);
			}
			else{
				pos[0] = 0; pos[2] = 14;
				drawRect(pos, PIGIT[HIGHSCORECOUNT - i]); }
				//hunderttausnder
				if(digits[5]>0){
					pos[0] = (2 - positionOffset) * 15; pos[2] = (2 - positionOffset) * 15 + 14;
					drawRect(pos, DIGIT[digits[5]]);}
				if(digits[4]>0 || digits[5]>0){
					pos[0] = (3 - positionOffset) * 15; pos[2] = (3 - positionOffset) * 15 + 14;
					drawRect(pos, DIGIT[digits[4]]);}
				if(digits[3]>0 || digits[4]>0 || digits[5]>0){
					pos[0] = (4 - positionOffset) * 15; pos[2] = (4 - positionOffset) * 15 + 14;
					drawRect(pos, DIGIT[digits[3]]);}
				if(digits[2]>0 || digits[3]>0 || digits[4]>0 || digits[5]>0){
					pos[0] = (5 - positionOffset) * 15; pos[2] = (5 - positionOffset) * 15 + 14;
					drawRect(pos, DIGIT[digits[2]]);}
				if(digits[1]>0 || digits[2]>0 || digits[3]>0 || digits[4]>0 || digits[5]>0){
					pos[0] = (6 - positionOffset) * 15; pos[2] = (6 - positionOffset) * 15 + 14;
					drawRect(pos, DIGIT[digits[1]]);}
				pos[0] = (7 - positionOffset) * 15; pos[2] = (7 - positionOffset) * 15 + 14;
				drawRect(pos, DIGIT[digits[0]]);
			break;
		}
		get_Digits(score,digits);
		int positionOffset = 0;
		for(int k=5; k>0; k--){
			if(digits[k] > 0) break;
			positionOffset++;
		}
		pos[0] = 0; pos[2] = 14;
		drawRect(pos, PIGIT[1]);
		if(digits[5]>0){
			pos[0] = (2 - positionOffset) * 15; pos[2] = (2 - positionOffset) * 15 + 14;
			drawRect(pos, DIGIT[digits[5]]);}
		if(digits[4]>0 || digits[5]>0){
			pos[0] = (3 - positionOffset) * 15; pos[2] = (3 - positionOffset) * 15 + 14;
			drawRect(pos, DIGIT[digits[4]]);}
		if(digits[3]>0 || digits[4]>0 || digits[5]>0){
			pos[0] = (4 - positionOffset) * 15; pos[2] = (4 - positionOffset) * 15 + 14;
			drawRect(pos, DIGIT[digits[3]]);}
		if(digits[2]>0 || digits[3]>0 || digits[4]>0 || digits[5]>0){
			pos[0] = (5 - positionOffset) * 15; pos[2] = (5 - positionOffset) * 15 + 14;
			drawRect(pos, DIGIT[digits[2]]);}
		if(digits[1]>0 || digits[2]>0 || digits[3]>0 || digits[4]>0 || digits[5]>0){
			pos[0] = (6 - positionOffset) * 15; pos[2] = (6 - positionOffset) * 15 + 14;
			drawRect(pos, DIGIT[digits[1]]);}
			pos[0] = (7 - positionOffset) * 15; pos[2] = (7 - positionOffset) * 15 + 14;
		drawRect(pos, DIGIT[digits[0]]);
	}
}

//is ok
void paint_Highscore(){
	uint8_t mydigits[HIGHSCORECOUNT][6]; // zur Umrechnung in Ziffern
	uint8_t digits[HIGHSCORECOUNT * 6]; // die paint-Schleifen verarbeiten die Digits in einer Kolonne
	int count = 0;
	for(int i=HIGHSCORECOUNT-1; i>=0; i--){ // digits mit Ziffern der Highscoreliste fuellen
		get_Digits(Highscore[i], mydigits[i]);
		for(int k=5; k>=0; k--){ digits[count] = mydigits[i][k]; count++; }
	}
	uint16_t color1 = 0xFFF0;
	uint16_t color2 = 0x0000;
	uint16_t color3 = 0xFFFF;
	setBgColor(color1);
	uint16_t xpos = (240 - 8*15) / 2;
	uint16_t ypos = (320 - HIGHSCORECOUNT * 30) / 2;
	uint16_t area[4];
	clearScreen();
	// mittlere Streifen
	setOrientation(HORIZONTAL);
	for(int x=0; x<HIGHSCORECOUNT; x++){
		area[0] = (ypos+5) + x * 30;
		area[1] = xpos;
		area[2] = area[0] + 4;
		area[3] = xpos + 8*15-1;
		paintRectGradient(area, color1, color2);
		area[0] = area[0] + 25;
		area[2] = area[0] + 4;
		paintRectGradient(area, color2, color1);
	}
	//oberer/unterer Rahmen
	setOrientation(VERTICAL);
	//oben links
	area[0] = xpos - 19;
	area[1] = (ypos - 49);
	area[2] = area[0] + 19;
	area[3] = area[1] + 49;
	paintRectGradient(area, color1, color2);
	//unten links
	area[1] = ypos + HIGHSCORECOUNT*30 - 6;
	area[3] = area[1] + 4;
	paintRectGradient(area, color1, color2);
	//unten rechts
	area[0] = xpos + 8*15-1;
	area[2] = area[0] + 19;
	paintRectGradient(area, color2, color1);
	//oben rechts
	area[1] = ypos - 49;
	area[3] = area[1] + 49;
	paintRectGradient(area, color2, color1);
	//ganz oben
	area[0] = xpos; area[1] = ypos -49; area[2] = xpos + 8*15; area[3] = area[1] + 5;
	paintRect(area, 0x0000);
	//unten mitte
	area[1] = ypos + HIGHSCORECOUNT*30 - 10; area[3] = area[1] + 8;
	paintRect(area, 0x0000);
	//oben mitte
	area[0] = xpos; area[1] = ypos - 43; area[2] = xpos + 8*15; area[3] = area[1] + 30;
	paintRect(area, color3);
	area[1] = area[3]+1; area[3] = area[1] + 9;
	paintRect(area, color2);

	area[0] = xpos; area[1] = ypos - 43; area[2] = xpos + 15; area[3] = area[1] + 30;
	paintRectGradient(area, color2, color1);
	area[0] = xpos + 7*15; area[2] = area[0] + 15;
	paintRectGradient(area, color1, color2);

	//rechter/linker Rahmen
	for(int y=0; y<HIGHSCORECOUNT; y++){
		area[0] = xpos - 19;
		area[1] = (ypos - 5) + y*30;
		area[2] = area[0] + 19;
		area[3] = area[1] + 28;
		paintRectGradient(area, color1, color2);
		area[0] = xpos + 8*15-1;
		area[2] = area[0] + 19;
		paintRectGradient(area, color2, color1);
	}

	//paint Scores
	int digicount = 0;
	for(int y=0; y<HIGHSCORECOUNT; y++){
		area[0] = xpos; area[2] = xpos + 14;
		area[1] = ypos + y*30;  area[3] = area[1] + 19;
		drawRect(area, PIGIT[y+1]);

		int leerstellen = 0;
		for(int x=digicount; x<digicount+6; x++){
			if(digits[x]>0) break;
			leerstellen++;
		}
		if(leerstellen > 5)leerstellen = 5;
		for(int x=0; x<7; x++){
			area[0] = xpos + (x+1) * 15; area[2] = area[0] + 14;
//			area[1] = ypos + y * 30; area[3] = area[1] + 19;
			if(x<1){ drawRect(area, DIGIT[10]); }
			else {
				if(x>(leerstellen)){ drawRect(area, DIGIT[digits[digicount]]); }
				else{ drawRect(area, DIGIT[10]); }
				digicount++;
			}
		}
	}
	sleep_ms(750); //Damit der Highscore nicht gleich verschwindet!
}
