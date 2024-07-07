// Artifical Horizon Demo
// Original demo written by Bodmer for 160x128 TFT display
// Different rendering approach and optimization for high resolution LCDs by Pawel A. Hernik
// YT video: https://youtu.be/hTFXPgVk5DA

// Required RRE Font library:
// https://github.com/cbm80amiga/RREFont
// ST7789_AVR library:
// https://github.com/cbm80amiga/ST7789_AVR

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
#define SCR_HT 240
ST7789_AVR tft = ST7789_AVR(TFT_DC, TFT_RST, TFT_CS);

#include "RREFont.h"
#include "rre_digitssimple5x7.h"
#include "rre_digitssimple5x7bg.h"
RREFont font;
// needed for RREFont library initialization, define your fillRect
void customRect(int x, int y, int w, int h, int c) { return tft.fillRect(x, y, w, h, c); }

#define FRAME_TIME 10 // in miliseconds, 10 ms = limit to max 100 fps

#define BROWN      RGBto565(150,50,0)
#define SKY_BLUE   0x02B5 //0x0318 //0x039B //0x34BF
#define DARK_RED   0x8000
#define DARK_GREY  0x39C7

#define XC 120   // x coord of centre of horizon
#define YC 120   // y coord of centre of horizon
#define HOR 350  // Horizon vector line length >240*1.41

#define DEG2RAD 0.0174532925

int last_roll = 0; // the whole horizon graphic
int last_pitch = 0;
int test_roll = 0;
int delta = 0;
unsigned long drawStart = 0;

void setup(void)
{
  Serial.begin(115200);
  tft.init(SCR_WD,SCR_HT);
  //tft.setRotation(2);
  font.init(customRect, SCR_WD, SCR_HT); // custom fillRect function and screen width and height values

  tft.fillRect(0,        0, SCR_WD, SCR_HT/2, SKY_BLUE);
  tft.fillRect(0, SCR_HT/2, SCR_WD, SCR_HT/2, BROWN);
  //drawHorizon(0, 0);

  // Test roll and pitch
  testRoll();
  testPitch();
}

int16_t ox0=-1,oy0,ox1,oy1,ox2,oy2;

void loop()
{
  // Roll is in degrees in range +/-180
  int roll = random(361) - 180;
  // Pitch is in y coord (pixel) steps
  // Maximum pitch shouls be in range +/- 90 with HOR = 172
  int pitch = random (181) - 90;
  updateHorizon(roll, pitch);
    
// testRoll();
}

// #########################################################################
// Update the horizon with a new roll (angle in range -180 to +180)
// #########################################################################

void updateHorizon(int roll, int pitch)
{
  int delta_pitch = 0;
  int pitch_error = 0;
  int delta_roll  = 0;
  while ((last_pitch != pitch) || (last_roll != roll)) {
    delta_pitch = 0;
    delta_roll  = 0;

    if (last_pitch < pitch) {
      delta_pitch = 1;
      pitch_error = pitch - last_pitch;
    }

    if (last_pitch > pitch) {
      delta_pitch = -1;
      pitch_error = last_pitch - pitch;
    }

    if (last_roll < roll) delta_roll  = 1;
    if (last_roll > roll) delta_roll  = -1;

    if (delta_roll == 0) {
      if (pitch_error > 1) delta_pitch *= 2;
    }
    drawHorizon(last_roll + delta_roll, last_pitch + delta_pitch);
    drawInfo();
  }
}

// --------------------
// optimized rendering
#define swap(a, b) { int16_t t = a; a = b; b = t; }

int16_t lastPos[SCR_HT];
int16_t lastPos2[SCR_HT];
int16_t lastSteep,steep;

void drawDiff(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, uint16_t sky, uint16_t ground, int roll, int pitch)
{
  int lineWd=1;
  int16_t dx, dy;
  steep = abs(y1-y0)>abs(x1-x0);
  if(steep) { swap(x0,y0); swap(x1,y1); }
  if(x0>x1) { swap(x0,x1); swap(y0,y1); }
  dx = x1 - x0;
  dy = abs(y1-y0);

  int16_t err = dx / 2;
  int16_t ys = (y0<y1) ? 1 : -1;
  int yy,yh,hh;

  if(steep!=lastSteep) {
    int d = 120-pitch;
    for(int i=d-1;i>=0;i--)
      if(lastPos[i]>=0 && lastPos[i]<240) lastPos2[lastPos[i]]=i;

    for(int i=d;i<240;i++)
      if(lastPos[i]>=0 && lastPos[i]<240) lastPos2[lastPos[i]]=i;

    hh=min(lastPos[0],lastPos[239]);
    //for(int i=0;i<hh;i++) lastPos2[i]=lastPos2[lastPos[0]];
    for(int i=0;i<hh;i++) lastPos2[i]=lastPos2[hh]<120?0:239;
    hh=max(lastPos[0],lastPos[239]);
    //for(int i=hh;i<240;i++) lastPos2[i]=lastPos2[lastPos[239]];
    for(int i=hh;i<240;i++) lastPos2[i]=lastPos2[hh]<120?0:239;
  }
  for(; x0<=x1; x0++) {
    if(x0>=0 && x0<SCR_HT ) {
      if(steep)  { // 46..90+44
        if(steep==lastSteep) {
          if(y0>lastPos[x0]) tft.drawFastHLine(lastPos[x0],x0, y0-lastPos[x0], ground); else
          if(y0<lastPos[x0]) tft.drawFastHLine(y0+lineWd,x0, lastPos[x0]-y0, sky);
        } else {
          if(y0>lastPos2[x0]) tft.drawFastHLine(lastPos2[x0],x0, y0-lastPos2[x0], ground); else
          if(y0<lastPos2[x0]) tft.drawFastHLine(y0+lineWd,x0, lastPos2[x0]-y0, sky);
        }
        tft.drawFastHLine(y0, x0,lineWd, color);
        lastPos[x0]=y0;
      } else {
        if(steep==lastSteep) {
          if(y0>lastPos[x0]) tft.drawFastVLine(x0, lastPos[x0], y0-lastPos[x0], sky); else
          if(y0<lastPos[x0]) tft.drawFastVLine(x0, y0+lineWd, lastPos[x0]-y0, ground);
        } else {
          if(y0>lastPos2[x0]) tft.drawFastVLine(x0, lastPos2[x0], y0-lastPos2[x0], sky); else
          if(y0<lastPos2[x0]) tft.drawFastVLine(x0, y0+lineWd, lastPos2[x0]-y0, ground);
        }
        tft.drawFastVLine(x0, y0,lineWd, color);
        lastPos[x0]=y0;
      }
    } 
    err -= dy;
    if(err<0) { y0 += ys; err += dx; }
  }
  lastSteep = steep;
}

// full screen rendering
void drawFull(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, uint16_t sky, uint16_t ground)
{
  int lineWd=1;
  int16_t steep = abs(y1-y0)>abs(x1-x0);
  if(steep) { swap(x0,y0); swap(x1,y1); }
  if(x0>x1) { swap(x0,x1); swap(y0,y1); }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1-y0);

  int16_t err = dx / 2;
  int16_t ystep = (y0<y1) ? 1 : -1;
  
  for(; x0<=x1; x0++) {
    if(x0>=0 && x0<SCR_HT && y0>=0 && y0<SCR_WD ) {
      if(steep)  {
        tft.drawFastHLine(0,x0, y0, ground); 
        tft.drawFastHLine(y0+lineWd,x0, SCR_HT-y0-lineWd, sky);
        tft.drawFastHLine(y0, x0,lineWd, color);
      } else {
        tft.drawFastVLine(x0, 0, y0, sky); 
        tft.drawFastVLine(x0, y0+lineWd, SCR_HT-y0-lineWd, ground);
        tft.drawFastVLine(x0, y0,lineWd, color);
      }
    }
    err -= dy;
    if(err < 0) { y0 += ystep; err += dx; }
  }
}

// --------------------
void drawHorizon(int roll, int pitch)
{
  // Calculate coordinates for line start
  float sx = cos(roll * DEG2RAD);
  float sy = sin(roll * DEG2RAD);
  int16_t x0 = sx * HOR;
  int16_t y0 = sy * HOR;
  uint16_t c1=SKY_BLUE;
  uint16_t c2=BROWN;

  if((roll>=90+45 && roll<315) || roll<-45) swap(c1,c2);
  //c1=random(60000);  c2=random(60000);
  drawStart = millis();
  drawDiff(XC - x0, YC - y0 - pitch,   XC + x0, YC + y0 - pitch, WHITE,c1,c2,  roll, pitch);
  //drawFull(XC - x0, YC - y0 - pitch,   XC + x0, YC + y0 - pitch, WHITE,c1,c2 );

  last_roll = roll;
  last_pitch = pitch;
}

// Draw overlay information

int ll=90,ls=70;
int dy=20,llh=ll/2,lsh=ls/2;

void drawLines(void)
{
  int x0=XC;
  int y0=YC;

  // Level wings graphic
  tft.fillRect(x0 - 1, y0 - 1, 3, 3, RED);
  tft.drawFastHLine(x0 - llh-6, y0, llh, RED);
  tft.drawFastHLine(x0 + 6, y0, llh, RED);
  tft.drawFastVLine(x0 - 6, y0, 6, RED);
  tft.drawFastVLine(x0 + 6, y0, 6, RED);

  // Pitch scale
  tft.drawFastHLine(x0 - llh,   y0 - dy*4, ll, WHITE);
  tft.drawFastHLine(x0 - lsh,   y0 - dy*3, ls, WHITE);
  tft.drawFastHLine(x0 - llh,   y0 - dy*2, ll, WHITE);
  tft.drawFastHLine(x0 - lsh,   y0 - dy*1, ls, WHITE);

  tft.drawFastHLine(x0 - lsh,   y0 + dy*1, ls, WHITE);
  tft.drawFastHLine(x0 - llh,   y0 + dy*2, ll, WHITE);
  tft.drawFastHLine(x0 - lsh,   y0 + dy*3, ls, WHITE);
  tft.drawFastHLine(x0 - llh,   y0 + dy*4, ll, WHITE);
}

void drawLabels(void)
{
  int x0=XC;
  int y0=YC;
  font.setColor(WHITE);
  font.setFont(&rre_digitssimple5x7);
  font.setScale(2,2);
  //font.setSpacing(2);
  font.setFontMinWd(5);
  font.printStr(x0-ll/2-6*4-2, y0 - dy*4-7, "80");
  font.printStr(x0+ll/2+4,     y0 - dy*3-7, "60");
  font.printStr(x0-ll/2-6*4-2, y0 - dy*2-7, "40");
  font.printStr(x0+ll/2+4,     y0 - dy*1-7, "20");
  
  font.printStr(x0+ll/2+4,     y0 + dy*1-7, "20");
  font.printStr(x0-ll/2-6*4-2, y0 + dy*2-7, "40");
  font.printStr(x0+ll/2+4,     y0 + dy*3-7, "60");
  font.printStr(x0-ll/2-6*4-2, y0 + dy*4-7, "80");
}

int fps=0, fpsAvg=0, fpsCnt=0;
uint16_t fpsCol = RGBto565(190,190,190);
uint16_t fpsBg =  RGBto565(60,60,60);

void drawFPS(void)
{
  int x=104,y=4;
  int fpsFr=4;
  int fpsCur=1000.0/(millis()-drawStart);
  fpsAvg+=fpsCur;
  //Serial.println(fpsCur);  Serial.println(fps);
  if(++fpsCnt>=5) {
    fps=fpsAvg/fpsCnt;
    fpsAvg=fpsCnt=0;
  }
  tft.fillRect(x, y,           12*2+2*fpsFr, fpsFr, fpsBg);
  tft.fillRect(x, y+7*2+fpsFr, 12*2+2*fpsFr, fpsFr, fpsBg);
  tft.fillRect(x,            y+fpsFr, fpsFr, 7*2, fpsBg);
  tft.fillRect(x+12*2+fpsFr, y+fpsFr, fpsFr, 7*2, fpsBg);
  tft.fillRect(x+5*2+fpsFr,  y+fpsFr,     4, 7*2, fpsBg);
  char buf[6];
  snprintf(buf,5,"%02d",fps>99 ? 99 : fps);
  font.setFont(&rre_digitssimple5x7);
  font.setScale(2,2);
  font.setColor(fpsCol);
  font.drawChar(x+fpsFr,y+fpsFr, buf[0]);
  font.drawChar(x+fpsFr+7*2,y+fpsFr, buf[1]);

  font.setFont(&rre_digitssimple5x7bg);
  font.setScale(2,2);
  font.setColor(fpsBg);
  font.drawChar(x+fpsFr,y+fpsFr, buf[0]);
  font.drawChar(x+fpsFr+7*2,y+fpsFr, buf[1]);
}


void drawInfo(void)
{
  drawLines();
  drawLabels();

  while(millis()<drawStart+FRAME_TIME);  // limit fps on faster MCUs
  drawFPS();
}

// #########################################################################
// Function to generate roll angles for testing only
// #########################################################################

int rollGenerator(int maxroll)
{
  // Synthesize a smooth +/- 50 degree roll value for testing
  delta++; if (delta >= 360) test_roll = 0;
  test_roll = (maxroll + 1) * sin((delta) * DEG2RAD);

  // Clip value so we hold roll near peak
  if (test_roll >  maxroll) test_roll =  maxroll;
  if (test_roll < -maxroll) test_roll = -maxroll;

  return test_roll;
}

// #########################################################################
// Function to generate roll angles for testing only
// #########################################################################

void testRoll(void)
{
  Serial.println(F("Test Roll"));
  //for(int a = 360; a >= 0; a-=1) {
  //for(int a = 0; a < 360; a+=2) {
  for(int a = 0; a < 360; a+=1) {
    //updateHorizon(rollGenerator(180), 0);
    //updateHorizon(rollGenerator(30)+90, 0);
    //updateHorizon(rollGenerator(160), 0);
    //updateHorizon(a, 0);
    drawHorizon(a, 0);
    drawInfo();
  }
}

// #########################################################################
// Function to generate pitch angles for testing only
// #########################################################################

void testPitch(void)
{
  Serial.println(F("Test Pitch"));
  for (int p =   0; p >-85; p--) updateHorizon(0, p);
  for (int p = -85; p < 85; p++) updateHorizon(0, p);
  for (int p =  85; p >  0; p--) updateHorizon(0, p);
}
 
