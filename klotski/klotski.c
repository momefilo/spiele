// momefilo Desing
#include "../include/libs/ili9341/ili9341.h"
#include "../include/libs/flash/flash.h"
#include "../include/melodys.h"
#include "../include/buttons.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "klotski.h"

#define MAXLEVEL 9
#include <stdio.h>
//	printf("KLOTSKI")

uint8_t *Levelnames[] = {"nur 18 Zuege","nur 28 Zuege","nur 38 Zuege","nur 48 Zuege","nur 58 Zuege",
	"nur 68 Zuege","nur 78 Zuege","nur 88 Zuege","nur 98 Zuege"};
uint32_t Klotz_FlashOffset, KlotskiScore[MAXLEVEL][5];
uint8_t *Klotz_FlashContent;
uint K0_offset, K1_offset, K2_offset, K3_offset, K4_offset, K5_offset, K6_offset, K7_offset;
void klotski_flashInit(){
	uint K0_size = KLOTS_0_LEN / FLASH_PAGE_SIZE +1, K1_size = KLOTS_1_LEN / FLASH_PAGE_SIZE +1;
	uint K2_size = KLOTS_2_LEN / FLASH_PAGE_SIZE +1, K3_size = KLOTS_3_LEN / FLASH_PAGE_SIZE +1;
	uint K4_size = KLOTS_4_LEN / FLASH_PAGE_SIZE +1, K5_size = KLOTS_5_LEN / FLASH_PAGE_SIZE +1;
	uint K6_size = KLOTS_6_LEN / FLASH_PAGE_SIZE +1, K7_size = KLOTS_7_LEN / FLASH_PAGE_SIZE +1;
	uint SectorSum;
	uint PageSum;
	K0_offset = 1;
	K1_offset = K0_size * FLASH_PAGE_SIZE+1;
	K2_offset = K1_offset + K1_size * FLASH_PAGE_SIZE;
	K3_offset = K2_offset + K2_size * FLASH_PAGE_SIZE;
	K4_offset = K3_offset + K3_size * FLASH_PAGE_SIZE;
	K5_offset = K4_offset + K4_size * FLASH_PAGE_SIZE;
	K6_offset = K5_offset + K5_size * FLASH_PAGE_SIZE;
	K7_offset = K6_offset + K6_size * FLASH_PAGE_SIZE;
	PageSum = K0_size + K1_size + K2_size + K3_size +  K4_size + K5_size + K6_size + K7_size;
	SectorSum = PageSum / 16 + 2;
	if(SectorSum < 200) SectorSum = 200;
	Klotz_FlashOffset = (PICO_FLASH_SIZE_BYTES - SectorSum * FLASH_SECTOR_SIZE)-2049;
	Klotz_FlashContent = (uint8_t *) (XIP_BASE + Klotz_FlashOffset);
}
uint8_t *getKlots(uint8_t klotsId){
	if(klotsId == 0) return &Klotz_FlashContent[K0_offset];
	else if(klotsId == 1) return &Klotz_FlashContent[K1_offset];
	else if(klotsId == 2) return &Klotz_FlashContent[K2_offset];
	else if(klotsId == 3) return &Klotz_FlashContent[K3_offset];
	else if(klotsId == 4) return &Klotz_FlashContent[K4_offset];
	else if(klotsId == 5) return &Klotz_FlashContent[K5_offset];
	else if(klotsId == 6) return &Klotz_FlashContent[K6_offset];
	else if(klotsId == 7) return &Klotz_FlashContent[K7_offset];
}
struct _Item{
	uint8_t id;
	uint8_t x;
	uint8_t y;
	uint8_t w;
	uint8_t h;
	uint8_t klots_id;
} Items[MAXLEVEL][15];
uint8_t Level;
uint8_t NewPoint[2];
int8_t GameArea4x5[MAXLEVEL][4][5];
uint16_t MoveCount;
uint8_t LastItem, LastPos[2];
void init_Areas(){
	//area 0 nur 18 Zuege
	uint8_t i = 1;
	for(uint8_t y=0; y<5; y++){
		if(y<2){
			GameArea4x5[0][0][y] = i++;
			GameArea4x5[0][3][y] = i++;
			GameArea4x5[0][1][y] = 15;
			GameArea4x5[0][2][y] = 15;
		}
		else if(y>1 && y<4){
			GameArea4x5[0][0][y] = i++;
			GameArea4x5[0][1][y] = i++;
			GameArea4x5[0][2][y] = i++;
			GameArea4x5[0][3][y] = i++;
		}
		else if(y>3){
			GameArea4x5[0][1][y] = 0;
			GameArea4x5[0][2][y] = 0;
			GameArea4x5[0][0][y] = i++;
			GameArea4x5[0][3][y] = i++;
		}
		for(uint8_t x=0; x<4; x++){
			uint8_t itemId = y*4+x+1;
			if(itemId>19) itemId = itemId - 6;
			else if(itemId>7) itemId = itemId - 4;
			else if(itemId>3) itemId = itemId - 2;
			Items[0][itemId - 1].id = itemId;
			Items[0][itemId - 1].x = x;
			Items[0][itemId - 1].y = y;
			Items[0][itemId - 1].w = 0;
			Items[0][itemId - 1].h = 0;
			Items[0][itemId - 1].klots_id = 0;
		}
	}
	//area 1 Gaensebluemchen
	i = 3;
	for(uint8_t y=0; y<5; y++){
		if(y<2){
			GameArea4x5[1][1][y] = 15;
			GameArea4x5[1][2][y] = 15;
			GameArea4x5[1][0][y] = 1;
			GameArea4x5[1][3][y] = 2;
		}
		else if(y>1 && y<4){
			GameArea4x5[1][0][y] = i++;
			GameArea4x5[1][1][y] = i++;
			GameArea4x5[1][2][y] = i++;
			GameArea4x5[1][3][y] = i++;
		}
		else if(y>3){
			GameArea4x5[1][0][y] = i++;
			GameArea4x5[1][3][y] = i++;
			GameArea4x5[1][1][y] = 0;
			GameArea4x5[1][2][y] = 0;
		}
	}
	i = 3;
	for(uint8_t y=2; y<4; y++){
		for(uint8_t x=0; x<4; x++){
			Items[1][i-1].id = i;
			Items[1][i-1].x = x;
			Items[1][i-1].y = y;
			Items[1][i-1].w = 0;
			Items[1][i-1].h = 0;
			Items[1][i-1].klots_id = 0;
			i++;
		}
	}
	Items[1][10].id = 11;
	Items[1][10].x = 0;
	Items[1][10].y = 4;
	Items[1][10].w = 0;
	Items[1][10].h = 0;
	Items[1][10].klots_id = 0;
	Items[1][11].id = 12;
	Items[1][11].x = 3;
	Items[1][11].y = 4;
	Items[1][11].w = 0;
	Items[1][11].h = 0;
	Items[1][11].klots_id = 0;
	Items[1][0].id = 1;
	Items[1][0].x = 0;
	Items[1][0].y = 0;
	Items[1][0].w = 0;
	Items[1][0].h = 1;
	Items[1][0].klots_id = 2;
	Items[1][1].id = 2;
	Items[1][1].x = 3;
	Items[1][1].y = 0;
	Items[1][1].w = 0;
	Items[1][1].h = 1;
	Items[1][1].klots_id = 2;
	//area 2 Veilchen
	i = 4;
	for(uint8_t y=0; y<5; y++){
		if(y<2){
			GameArea4x5[2][1][y] = 15;
			GameArea4x5[2][2][y] = 15;
			GameArea4x5[2][0][y] = 1;
			GameArea4x5[2][3][y] = 2;
		}
		else if(y>1 && y<4){

			GameArea4x5[2][0][y] = 3;
			GameArea4x5[2][1][y] = i++;
			GameArea4x5[2][2][y] = i++;
			GameArea4x5[2][3][y] = i++;
		}
		else if(y>3){
			GameArea4x5[2][0][y] = i++;
			GameArea4x5[2][3][y] = i++;
			GameArea4x5[2][1][y] = 0;
			GameArea4x5[2][2][y] = 0;
		}
	}
	//area 3 Mohnblume
	i = 4;
	for(uint8_t y=0; y<5; y++){
		if(y<2){
			GameArea4x5[3][1][y] = 15;
			GameArea4x5[3][2][y] = 15;
			GameArea4x5[3][0][y] = 1;
			GameArea4x5[3][3][y] = 2;
		}
		else if(y==2){

			GameArea4x5[3][0][y] = i++;
			GameArea4x5[3][1][y] = 3;
			GameArea4x5[3][2][y] = 3;
			GameArea4x5[3][3][y] = i++;
		}
		else if(y>2){
			GameArea4x5[3][0][y] = i++;
			GameArea4x5[3][3][y] = i++;
			if(y<4){
				GameArea4x5[3][1][y] = i++;
				GameArea4x5[3][2][y] = i++;
			}else{
				GameArea4x5[3][1][y] = 0;
				GameArea4x5[3][2][y] = 0;
			}
		}
	}
	for(uint m=0; m<MAXLEVEL; m++){
		Items[m][14].id = 15;
		Items[m][14].x = 1;
		Items[m][14].y = 0;
		Items[m][14].w = 1;
		Items[m][14].h = 1;
		Items[m][14].klots_id = 6;
	}
	for(int i=0;i<1;i++){
//		printf("Area [%d]\n",i);
		for(int y=0;y<5;y++){
//			for(int x=0;x<4;x++) printf("%d |",GameArea4x5[i][x][y]);
//			printf("\n");
		}
	}
	for(int i=0; i<15; i++){
//		printf("%d, %d, %d, %d, %d\n",Items[0][i].id,Items[0][i].x,Items[0][i].y,Items[0][i].w,Items[0][i].h);
	}
}

void paintItem(uint8_t itemId, uint8_t selected){
	uint16_t x = Items[Level][itemId].x * 50 + 20, y = Items[Level][itemId].y * 50 + 25;
	uint16_t w = Items[Level][itemId].w * 50, h = Items[Level][itemId].h * 50;
	uint16_t area[] = { x, y, x + 50 + w - 1, y + 50 + h - 1};
	uint8_t klotsid = Items[Level][itemId].klots_id + selected;
//	uint32_t flags = save_and_disable_interrupts();
	switch(klotsid){
		case 0: drawRect(area,getKlots(0)); break;
		case 1: drawRect(area,getKlots(1)); break;
		case 2: drawRect(area,getKlots(2)); break;
		case 3: drawRect(area,getKlots(3)); break;
		case 4: drawRect(area,getKlots(4)); break;
		case 5: drawRect(area,getKlots(5)); break;
		case 6: drawRect(area,getKlots(6)); break;
		case 7: drawRect(area,getKlots(7)); break;
		default: break;
	}
//	restore_interrupts(flags);
//	drawRect(area, data);
}
void selectItem(uint8_t itemId){paintItem(itemId, 1);}
void deselectItem(uint8_t itemId){paintItem(itemId,0);}
void paintBorder(){
	uint16_t color1 = 0x8410, color2 = 0x0000;
	uint16_t area[] = {14 ,0 , 19, 319};
	paintRectGradient(area, color2, color1);
	area[0] = 220; area[1] = 0; area[2] = 225; area[3] = 319;
	paintRectGradient(area, color1, color2);

	area[0] = 0; area[1] = 0; area[2] = 13; area[3] = 319;
	paintRect(area, color2);
	area[0] = 226; area[1] = 0; area[2] = 239; area[3] = 319;
	paintRect(area, color2);

	setOrientation(HORIZONTAL);
	area[0] = 40; area[1] = 17; area[2] = 45; area[3] = 223;
	paintRectGradient(area, color2, color1);
	area[0] = 314; area[1] = 20; area[2] = 319; area[3] = 220;
	paintRectGradient(area, color1, color2);
	setOrientation(VERTICAL);

	area[0] = 0; area[1] = 280; area[2] = 239; area[3] = 319;
	paintRect(area, color2);

	area[0] = 95; area[1] = 280; area[2] = 104; area[3] = 289;
	paintRect(area, 0x07E0);
	area[0] = 136; area[1] = 280; area[2] = 145; area[3] = 289;
	paintRect(area, 0x07E0);

	//text
	area[0] = 10; area[1] = 300; area[2] = 19; area[3] = 309;
	paintRect(area, 0xFFE0);
	setFgColor(0xFFE0);
	setBgColor(0x0000);
	uint16_t pos[] = {25, 300};
	writeText16x16(pos, "Taste Zurueck", 13, false, false);
}
void paintMovecount(){
	uint16_t color1 = 0x8410, color2 = 0x0010;
	uint16_t area[] = {20 ,5 , 29, 24};
	paintRectGradient(area, color1, color2);
	area[0] = 210; area[1] = 5; area[2] = 219; area[3] = 24;
	paintRectGradient(area, color2, color1);
	area[0] = 30; area[1] = 5; area[2] = 209; area[3] = 24;
	paintRect(area, color2);
	setBgColor(color2);
	setFgColor(0xE71C);
	if(MoveCount < 10){
		char text[7];
		sprintf(text, "%d Zuege", MoveCount);
		uint16_t pos[] = {65,7};
		writeText16x16(pos, text, 7, false, false);
	}
	else if(MoveCount < 100){
		char text[8];
		sprintf(text, "%d Zuege", MoveCount);
		uint16_t pos[] = {57,7};
		writeText16x16(pos, text, 8, false, false);
	}
	else{
		char text[9];
		sprintf(text, "%d Zuege", MoveCount);
		uint16_t pos[] = {49,7};
		writeText16x16(pos, text, 9, false, false);
	}
}

bool paintMenu(){
	setBgColor(0x07E0);
	setFgColor(0x0007);
	clearScreen();
	uint16_t y = 320 / (MAXLEVEL + 1);
	uint16_t x = 20 ;
	for(uint16_t i=1; i<=MAXLEVEL; i++){
		uint16_t pos[] = {x, i*y-y/2};
		writeText16x16(pos, Levelnames[i-1], 12, false, false);
	}
	uint16_t pos[] = {70, 300};
	setBgColor(0x0007);
	setFgColor(0x07E0);
	writeText16x16(pos, "Zurueck", 7, false, false);
	while(1){
		uint16_t *pixPoint = ili9341_getTouch();
		while(pixPoint[0]<0xFFFF){
			pixPoint = ili9341_getTouch();
			for(uint8_t i=1; i<=MAXLEVEL; i++){
				uint16_t x0 = 0, x1 = 219, y0 = (i*y - y/2), y1 = (i*y - y/2) + y;
				uint16_t area[] = {x0,y0,x0+15,y0+15};
				if(pixPoint[0]>x0 && pixPoint[0]<x1 && pixPoint[1]>y0 && pixPoint[1]<y1){
					Level = i - 1;
					paintRect(area, 0x001F);
//					sleep_ms(200);
					return true;
				}
				else if(pixPoint[0]>50 && pixPoint[0]<170 && pixPoint[1]>300 && pixPoint[1]<319) return false;
			}
		}

	}
}

bool moveItem(uint8_t itemId, int8_t *posDiff){
	uint8_t x = Items[Level][itemId].x, y = Items[Level][itemId].y;
	uint8_t w = Items[Level][itemId].w, h = Items[Level][itemId].h;
	uint8_t klotsId = Items[Level][itemId].klots_id;
	// change GameArea
	for(uint8_t x=0; x<4; x++){
		for(uint8_t y=0; y<5; y++)
			if(GameArea4x5[Level][x][y] == itemId+1) GameArea4x5[Level][x][y] = 0;
	}
	for(uint8_t i=x; i<=x+w; i++){
		int8_t newx = i + posDiff[0];
		for(uint8_t k=y; k<=y+h; k++){
			int8_t newy = k + posDiff[1];
			GameArea4x5[Level][newx][newy] = itemId+1;
		}
	}
	//move Item
	for(uint i=0; i<50; i++){
		uint16_t xn = x * 50 + 20 + i * posDiff[0];
		uint16_t yn = y * 50 + 25 + i * posDiff[1];
		if(klotsId < 2){
			uint16_t area[4];
			area[0] = xn; area[1] = yn; area[2] = xn+49; area[3] = yn + 49;
			drawRect(area,getKlots(klotsId+1));
			sleep_ms(4);
		}
		else if(klotsId < 4){
			uint16_t area[4];
			area[0] = xn; area[1] = yn; area[2] = xn+49; area[3] = yn + 99;
			drawRect(area,getKlots(klotsId+1));
			sleep_ms(2);
		}
		else if(klotsId < 6){
			uint16_t area[4];
			area[0] = xn; area[1] = yn; area[2] = xn+99; area[3] = yn + 49;
			drawRect(area,getKlots(klotsId+1));
		}
		else {
			uint16_t area[] = {xn + 98, yn, xn + 99, yn + 99};
			if(posDiff[1]>0){ //y-move
				area[0] = xn; area[1] = yn+98; area[2] = xn+99; area[3] = yn + 99;}
			paintRect(area, 0x8410);
			area[0] = xn; area[1] = yn; area[2] = xn+99; area[3] = yn + 99;
			drawRect(area,getKlots(klotsId+1));
		}
	}
	//change Item
	Items[Level][itemId].x = x + posDiff[0];
	Items[Level][itemId].y = y + posDiff[1];
	return true;
}

bool checkPosition(uint8_t itemId, int8_t *posDiff){
	uint8_t x = Items[Level][itemId].x, y = Items[Level][itemId].y;
	uint8_t w = Items[Level][itemId].w, h = Items[Level][itemId].h;
	for(uint8_t i=x; i<=x+w; i++){
		int8_t newx = i + posDiff[0];
		for(uint8_t k=y; k<=y+h; k++){
			int8_t newy = k + posDiff[1];
			if(newx<0 || newx>3 || newy<0 || newy>4
			||(GameArea4x5[Level][newx][newy]!=0 && GameArea4x5[Level][newx][newy]!=itemId+1)){
				return false;
			}
		}
	}
	return true;
}

uint8_t *getNewPos(uint16_t x, uint16_t y){
	uint16_t *pixPoint = ili9341_getTouch();
	while(pixPoint[0]<0xFFFF){
		pixPoint = ili9341_getTouch();
		if( (pixPoint[0]>x+3 || pixPoint[0]<x-3 || pixPoint[1]>y+3 || pixPoint[1]<y-3)
			&& pixPoint[0]>19 && pixPoint[0]<220 && pixPoint[1]>24 && pixPoint[1]<275){
			uint8_t newAreaPoint[] = {(pixPoint[0]-20)/50,(pixPoint[1]-25)/50};
			uint8_t areaPoint[] = {(x-20)/50,(y-25)/50};
			if(newAreaPoint[0] != areaPoint[0] || newAreaPoint[1] != areaPoint[1]){
				NewPoint[0] = newAreaPoint[0];
				NewPoint[1] = newAreaPoint[1];
				return NewPoint;
			}
		}
		sleep_ms(10);
	}
	NewPoint[0] = 0xFF;
	NewPoint[1] = 0xFF;
	return NewPoint;
}

bool play(){
	while(! gpio_get(BUTTON_D)){
		uint16_t *pixPoint = ili9341_getTouch();
		uint16_t px=pixPoint[0], py=pixPoint[1];
		if(px > 19 && px < 220 && py > 24 && py < 275){
			uint8_t areaPoint[] = {(px-20)/50,(py-25)/50};
			int8_t itemId = GameArea4x5[Level][areaPoint[0]][areaPoint[1]];
			if(itemId > 0){
				selectItem(itemId-1);
				uint8_t *newAreaPoint = getNewPos(px,py);
				if(newAreaPoint[0] != 0xFF){
					int8_t posDiff[2] = {newAreaPoint[0]-areaPoint[0], newAreaPoint[1]-areaPoint[1]};
					if(checkPosition(itemId - 1, posDiff))moveItem(itemId -1, posDiff);
					if(LastItem != itemId){
						LastItem = itemId;
						LastPos[0] = areaPoint[0];
						LastPos[1] = areaPoint[1];
						MoveCount ++;
					}
					else if( (LastPos[0] == newAreaPoint[0])
					&& (LastPos[1] == newAreaPoint[1])){
						MoveCount--;
						LastItem = 0;
						LastPos[0] = 255;
						LastPos[1] = 255;
					}
				}
				else deselectItem(itemId-1);
				paintMovecount();
				if(Items[Level][14].x == 1 && Items[Level][14].y == 3) return true;
			}
			sleep_ms(100);

		}
	}
	return false;
}

void win(){
	uint8_t rang = 0xFF;
	for(uint8_t i=0; i<5; i++){
		if(KlotskiScore[Level][i] > MoveCount){// && KlotskiScore[Level][i] < (1 << 31)){ //in Scoreliste
			for(uint8_t k=4; k>i; k--) KlotskiScore[Level][k] = KlotskiScore[Level][k-1];
			KlotskiScore[Level][i] = MoveCount;
			rang = i;
			break;
		}
	}
	uint32_t buf[MAXLEVEL * 5];
	uint8_t i = 0;
	for(uint8_t k=0; k<MAXLEVEL; k++){
		for(uint8_t l=0; l<5; l++){
			buf[i] = KlotskiScore[k][l];
			if(buf[i] < 1) buf[i] = (1 << 31);
			i++;
		}
	}
	flash_setDataRow(0, MAXLEVEL * 5 - 1, buf);
	setBgColor(0xFFE0);
	setFgColor(0x0007);
	clearScreen();
	uint16_t y = 320 / (7);
	uint16_t x = 20 ;
	for(uint16_t i=1; i<6; i++){
		uint16_t pos[] = {x, i*y-y/2};
		char text[12];
		sprintf(text, "Rang %d: %d",i, KlotskiScore[Level][i-1]);
		writeText16x16(pos, text, 12, false, false);
	}
	uint16_t pos[] = {70, 300};
	setBgColor(0x0007);
	setFgColor(0xFFE0);
	writeText16x16(pos, "Zurueck", 7, false, false);
	bool sprung = false;
	while(1){
		uint16_t *pixPoint = ili9341_getTouch();
		while(pixPoint[0]<0xFFFF){
			pixPoint = ili9341_getTouch();
			if(pixPoint[0]>50 && pixPoint[0]<170 && pixPoint[1]>300 && pixPoint[1]<319){
				sleep_ms(250);
				return;
			}
		}

	}
}

void klotski_init(uint8_t progId){
	buttons_init();
	ili9341_init();
	setOrientation(VERTICAL);
	ili9341_touch_init();
	melodys_init();
	flash_init(progId);
	klotski_flashInit();
	uint32_t *buf = flash_getData();
	uint8_t i = 0;
	for(uint8_t k=0; k<MAXLEVEL; k++){
		for(uint8_t l=0; l<5; l++){
			KlotskiScore[k][l] = buf[i];
			if(KlotskiScore[k][l] < 1) KlotskiScore[k][l] = (1 << 31);
			i++;
		}
	}
	while(1){
		bool wining = false;
		init_Areas();
		MoveCount = 0;
		LastItem = 0;
		LastPos[0] = 255;
		LastPos[1] = 255;
		NewPoint[0] = 0;
		NewPoint[1] = 0;
		if(! paintMenu()) break;
		uint16_t area[] = {0, 0, 239, 319};
		paintRect(area, 0x8410);
		for(int i=0; i<15;i++){
			if(Level<1 || i<12 || i>13) paintItem(i,0);
		}
		paintBorder();
		paintMovecount();
		if(play()){ wining =true;}
		if(wining) win();
	}
	uint16_t area[] = {0, 0, 239, 319};
	paintRect(area, 0x0000);
}
