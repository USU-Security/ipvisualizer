
/* function prototypes for the stuff in video.c */
#include "constants.h"

void videoinit();
int startvideo(const char*, int , int );
void writeframe(uint8_t imgdata[][COLORDEPTH], int , int );
void endvideo();
