//momefilo Desing
#ifndef klotski_h
#define klotski_h 1

#include "pico/stdlib.h"

#define KLOTS_0_LEN  (50*50*2+1)
#define KLOTS_1_LEN  (50*50*2+1)
#define KLOTS_2_LEN  (50*100*2+1)
#define KLOTS_3_LEN  (50*100*2+1)
#define KLOTS_4_LEN  (100*50*2+1)
#define KLOTS_5_LEN  (100*50*2+1)
#define KLOTS_6_LEN  (100*100*2+1)
#define KLOTS_7_LEN  (100*100*2+1)

void klotski_init(uint8_t progId);

#endif
