#include <stdio.h>
#include <stdlib.h>

int main(){
	int file_cnt = 8;
	char *data_names[] ={"1x1_n.data", "1x1_i.data", "1x2_n.data", "1x2_i.data",
		"2x1_n.data", "2x1_i.data", "2x2_n.data", "2x2_i.data"};
	int data_lens[] = {50*50*3, 50*100*3, 100*50*3, 100*100*3};
	char *data_lens_text[] = {"50*50*2+1", "50*100*2+1", "100*50*2+1", "100*100*2+1"};

	FILE *fileH = fopen("graphics.h", "w");
	if(fileH==0){printf("Dateifehler h\n");return 0;}
	fprintf(fileH, "//momefilo Desing\n");

	//5r6g5b pixelformat (3Ah =03h)
	for(int i=0; i<file_cnt; i++){
		//char filename[14];
		//sprintf(filename, "rang%d.data", i);
		FILE *fileData = fopen(data_names[i], "rb");
		if(fileData==0){printf("Dateifehler d\n");return 0;}
		unsigned char data[data_lens[i/2]];
		fread(data, 1, data_lens[i/2], fileData);
		fprintf(fileH, "#define KLOTS_%d_LEN  (%s)\n",i, data_lens_text[i/2]);
		fprintf(fileH, "static uint8_t KLOTS_%d[%s] ={\n",i, data_lens_text[i/2]);
		fprintf(fileH, "0X2C, // %d\n", i);
		//read 3 bytes and write 1 word
		for(int m=0; m<data_lens[i/2]-2; m=m+3){
			if(m%4==0&&m>0)fprintf(fileH, " ");
			if(m%24==0&&m>0)fprintf(fileH, "\n");
			//word 0x0000;//RRRR RGGG GGGB BBBB
			u_int16_t blue = (data[m+2] >>3) & 0x1F;
			u_int16_t green = ((data[m+1] >>2) & 0x3F)<<5;
			u_int16_t red = ((data[m] >>3) & 0x1F)<<11;
			u_int16_t sum = red | green | blue;
			fprintf(fileH, "0x%02X,0x%02X,", (sum&0xFF00)>>8, (sum&0x00FF));
		}
		fprintf(fileH, "};\n");
		fclose(fileData);
	}
	fclose(fileH);
}
