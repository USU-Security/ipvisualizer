
/* function prototypes for the stuff in video.c */

#define COLORDEPTH 4 

void videoinit();
int startvideo(const char*, int , int );
void writeframe(uint8_t imgdata[][COLORDEPTH], int , int );
void endvideo();
