// ST7789 library example
// Amiga Boing Ball Demo
// (c) 2019-24 Pawel A. Hernik
// YT video: https://youtu.be/KwtkfmglT-c
// Added support for LCD height 320,280,240
// Optimized ball refresh from 61 to 39ms

/*
ST7789 240x240 1.3" IPS (without CS pin) - only 4+2 wires required:
 #01 GND -> GND
 #02 VCC -> VCC (3.3V only!)
 #03 SCL -> D13/SCK
 #04 SDA -> D11/MOSI
 #05 RES -> D9 /PA0 or any digital (HW RESET is required to properly initialize LCD without CS)
 #06 DC  -> D10/PA1 or any digital
 #07 BLK -> NC

ST7789 240x280 1.69" IPS - only 4+2 wires required:
 #01 GND -> GND
 #02 VCC -> VCC (3.3V only!)
 #03 SCL -> D13/SCK
 #04 SDA -> D11/MOSI
 #05 RES -> optional
 #06 DC  -> D10 or any digital
 #07 CS  -> D9 or any digital
 #08 BLK -> VCC

ST7789 170x320 1.9" IPS - only 4+2 wires required:
 #01 GND -> GND
 #02 VCC -> VCC (3.3V only!)
 #03 SCL -> D13/SCK
 #04 SDA -> D11/MOSI
 #05 RES -> optional
 #06 DC  -> D10 or any digital
 #07 CS  -> D9 or any digital
 #08 BLK -> VCC

ST7789 240x320 2.0" IPS - only 4+2 wires required:
 #01 GND -> GND
 #02 VCC -> VCC (3.3V only!)
 #03 SCL -> D13/SCK
 #04 SDA -> D11/MOSI
 #05 RES -> optional
 #06 DC  -> D10 or any digital
 #07 CS  -> D9 or any digital
*/

#include <SPI.h>
#include <Adafruit_GFX.h>
#include "ST7789_AVR.h"

#define TFT_DC   10
#define TFT_CS    9  // with CS
#define TFT_RST  -1  // with CS
//#define TFT_CS  -1 // without CS
//#define TFT_RST  9 // without CS

#define SCR_WD 240
#define SCR_HT 320
ST7789_AVR lcd = ST7789_AVR(TFT_DC, TFT_RST, TFT_CS);

#include "ball.h"

uint16_t palette[16];
uint16_t line[SCR_WD];
uint16_t bgCol    = RGBto565(160,160,160);
uint16_t bgColS   = RGBto565(100,100,100);
uint16_t lineCol  = RGBto565(150,40,150);
uint16_t lineColS = RGBto565(90,20,90);

#define BALL_WD 64
#define BALL_HT 64
#define BALL_SWD SCR_WD
#define LINE_YS  20
#define LINE_XS1 30
#define LINE_XS2 6
#define SP 20

// support for 240, 280 and 320 line LCDs
#if SCR_HT==240
#define BALL_SHT 180
#elif SCR_HT==320
#define BALL_SHT 260
#elif SCR_HT==280
#define BALL_SHT 220
#endif

#define SHADOW 20
//#define SHADOW 0

// AVR stats:
// with shadow        - 60-61ms/17fps
// without shadow     - 55-56ms/18fps
// with shadow        - 38-40ms/26fps -> new partial line copying

void drawBall(int x, int y)
{
  int i,j,ii;
  for(j=0;j<BALL_HT;j++) {
    uint8_t v,*img = (uint8_t*)ball+16*2+6+j*BALL_WD/2+BALL_WD/2;
    int yy=y+j;
    //if(yy==LINE_YS      || yy==LINE_YS+1*SP || yy==LINE_YS+2*SP || yy==LINE_YS+3*SP || yy==LINE_YS+4*SP || yy==LINE_YS+5*SP || yy==LINE_YS+6*SP ||
    //   yy==LINE_YS+7*SP) {  // ugly but fast
    if(yy==LINE_YS      || yy==LINE_YS+1*SP || yy==LINE_YS+2*SP || yy==LINE_YS+3*SP || yy==LINE_YS+4*SP || yy==LINE_YS+5*SP || yy==LINE_YS+6*SP ||
       yy==LINE_YS+7*SP || yy==LINE_YS+8*SP || yy==LINE_YS+9*SP || yy==LINE_YS+10*SP || yy==LINE_YS+11*SP || yy==LINE_YS+12*SP) {  // ugly but fast 
    //if(((yy-LINE_YS)%SP)==0) {
      for(i=0;i<LINE_XS1;i++) line[i]=line[BALL_SWD-1-i]=bgCol;
      for(i=0;i<=BALL_SWD-LINE_XS1*2;i++) line[i+LINE_XS1]=lineCol;
    } else {
      for(i=0;i<BALL_SWD;i++) line[i]=bgCol;
      if(yy>LINE_YS) for(i=0;i<10;i++) line[LINE_XS1+i*SP]=lineCol;
    }
    for(i=BALL_WD-2;i>=0;i-=2) {
      v = pgm_read_byte(--img);
      if(v>>4)  {
        line[x+i+0] = palette[v>>4];
        #if SHADOW>0
        ii=x+i+0+SHADOW;
        if(ii<BALL_SWD) { if(line[ii]==bgCol) line[ii]=bgColS; else if(line[ii]==lineCol) line[ii]=lineColS; }
        #endif
      }
      if(v&0xf) {
        line[x+i+1] = palette[v&0xf];
        #if SHADOW>0
        ii=x+i+1+SHADOW;
        if(ii<BALL_SWD) { if(line[ii]==bgCol) line[ii]=bgColS; else if(line[ii]==lineCol) line[ii]=lineColS; }
        #endif
      }
    }
    //lcd.drawImage(0,yy,SCR_WD,1,line); // old full line copy
    int bx=x-2,bw=BALL_WD+SHADOW+2+2; // 2 pixels are safe enough
    if(bx<0) bx=0;
    if(bx+bw>SCR_WD) bw=SCR_WD-bx;
    lcd.drawImage(bx,yy,bw,1,line+bx); // optimized part line copy
  }
}

void setup() 
{
  Serial.begin(115200);
  lcd.init(SCR_WD,SCR_HT);
  //lcd.setRotation(2);
  lcd.fillScreen(bgCol);
  //lcd.invertDisplay(0);
  //lcd.idleDisplay(0);

  int i,o,numl=(SCR_HT==320?12:SCR_HT==240?8:10);
  uint16_t *pal = (uint16_t*)ball+3;
  for(i=0;i<16;i++) palette[i] = pgm_read_word(&pal[i]);
  for(i=0;i<10;i++) lcd.drawFastVLine(LINE_XS1+i*SP, LINE_YS, numl*SP, lineCol);
  for(i=0;i<=numl;i++) lcd.drawFastHLine(LINE_XS1, LINE_YS+i*SP, SCR_WD-LINE_XS1*2, lineCol);
  lcd.drawFastHLine(LINE_XS2,SCR_HT-LINE_YS, SCR_WD-LINE_XS2*2, lineCol);
  int dy=SCR_HT-LINE_YS-(LINE_YS+SP*numl);
  int dx=LINE_XS1-LINE_XS2;
  o=2*7*dx/dy;
  lcd.drawFastHLine(LINE_XS2+o,SCR_HT-LINE_YS-7*2, SCR_WD-LINE_XS2*2-o*2, lineCol);
  o=2*(7+6)*dx/dy;
  lcd.drawFastHLine(LINE_XS2+o,SCR_HT-LINE_YS-(7+6)*2, SCR_WD-LINE_XS2*2-o*2, lineCol);
  o=2*(7+6+4)*dx/dy;
  lcd.drawFastHLine(LINE_XS2+o,SCR_HT-LINE_YS-(7+6+4)*2, SCR_WD-LINE_XS2*2-o*2, lineCol);
  for(i=0;i<10;i++) lcd.drawLine(LINE_XS1+i*SP, LINE_YS+SP*numl, LINE_XS2+i*(SCR_WD-LINE_XS2*2)/9, SCR_HT-LINE_YS, lineCol);
  //delay(10000);
}

int anim=0, animd=2;
int x=0, y=0;
int xd=4, yd=1;
unsigned long ms;

void loop()
{
  ms=millis();
  for(int i=0;i<14;i++) {
    palette[i+1] = ((i+anim)%14)<7 ? WHITE : RED;
    //int c=((i+anim)%14); // with pink between white and red
    //if(c<6) palette[i+1]=WHITE; else if(c==6 || c==13) palette[i+1]=RGBto565(255,150,150); else palette[i+1]=RED;
  }
  //x=50;y=90;anim=0;
  drawBall(x,y);
  anim+=animd;
  if(anim<0) anim+=14;
  x+=xd; y+=yd;
  if(x<0) { x=0; xd=-xd; animd=-animd; }
  if(x>=BALL_SWD-BALL_WD) { x=BALL_SWD-BALL_WD; xd=-xd; animd=-animd; }
  if(y<0) { y=0; yd=-yd; }
  if(y>=BALL_SHT-BALL_HT) { y=BALL_SHT-BALL_HT; yd=-yd; }
  //ms=millis()-ms; Serial.println(ms);
}

