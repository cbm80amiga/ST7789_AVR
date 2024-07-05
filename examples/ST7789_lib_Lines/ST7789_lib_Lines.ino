// ST7789 library example
// (c) 2019-24 Pawel A. Hernik

/*
ST7789 240x240 1.3" IPS (without CS pin) - only 4+2 wires required:
 #01 GND -> GND
 #02 VCC -> VCC (3.3V only!)
 #03 SCL -> D13/SCK
 #04 SDA -> D11/MOSI
 #05 RES -> D9 or any digital (HW RESET is required to properly initialize LCD without CS)
 #06 DC  -> D10 or any digital
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
//#define TFT_CS    9  // with CS
//#define TFT_RST  -1  // with CS
#define TFT_CS  -1 // without CS
#define TFT_RST  9 // without CS

#define SCR_WD 240
#define SCR_HT 240
ST7789_AVR lcd = ST7789_AVR(TFT_DC, TFT_RST, TFT_CS);

void setup() 
{
  Serial.begin(9600);
  lcd.init();
  lcd.fillScreen(BLACK);
/*
  for(int i=0;i<128;i++) {
    lcd.fillRect(32*0,i,32,1,RGBto565(i*2,0,0));
    lcd.fillRect(32*1,i,32,1,RGBto565(0,i*2,0));
    lcd.fillRect(32*2,i,32,1,RGBto565(0,0,i*2));
    lcd.fillRect(32*3,i,32,1,RGBto565(i*2,i*2,i*2));
  }
  delay(2000);
*/
}

void circles()
{
  int x,y;
  x = 15+random(SCR_WD-30);
  y = 15+random(SCR_HT-30);
  lcd.fillCircle(x,y,15,random());
  lcd.drawCircle(x,y,16,YELLOW);
}

#define MAX_LINES 16
byte lx0[MAX_LINES];
byte lx1[MAX_LINES];
byte ly0[MAX_LINES];
byte ly1[MAX_LINES];
byte curLine=0;
int x0=40,y0=0;
int dx0=6,dy0=5;
int x1=80,y1=40;
int dx1=9,dy1=8;
int cc=0;
int numLines=12;

void animLines(int mode)
{
  uint16_t c;
  x0+=dx0;
  y0+=dy0;
  x1+=dx1;
  y1+=dy1;
  if(x0>SCR_WD-1) { dx0=-dx0; x0=SCR_WD-1; }
  if(x0<1) { dx0=-dx0; x0=0; }
  if(y0>SCR_HT-1) { dy0=-dy0; y0=SCR_HT-1; }
  if(y0<1) { dy0=-dy0; y0=0; }
  if(x1>SCR_WD-1) { dx1=-dx1; x1=SCR_WD-1; }
  if(x1<1) { dx1=-dx1; x1=0; }
  if(y1>SCR_HT-1) { dy1=-dy1; y1=SCR_HT-1; }
  if(y1<1) { dy1=-dy1; y1=0; }
  int i=curLine;
  if(mode&1) lcd.drawLine(lx0[i],ly0[i],lx1[i],ly1[i], 0);
  if(mode&2) lcd.drawLine(SCR_WD-1-lx0[i],ly0[i],SCR_WD-1-lx1[i],ly1[i],0);
  if(mode&4) lcd.drawLine(lx0[i],SCR_HT-1-ly0[i],lx1[i],SCR_HT-1-ly1[i],0);
  if(mode&8) lcd.drawLine(SCR_WD-1-lx0[i],SCR_HT-1-ly0[i],SCR_WD-1-lx1[i],SCR_HT-1-ly1[i],0);
  lx0[curLine]=x0;
  lx1[curLine]=x1;
  ly0[curLine]=y0;
  ly1[curLine]=y1;
  if(++curLine>=numLines) curLine=0;
  for(int i=0;i<numLines;i++) {
    c = lcd.rgbWheel(cc+i*25);
    if(mode&1) lcd.drawLine(lx0[i],ly0[i],lx1[i],ly1[i], c);
    if(mode&2) lcd.drawLine(SCR_WD-1-lx0[i],ly0[i],SCR_WD-1-lx1[i],ly1[i],c);
    if(mode&4) lcd.drawLine(lx0[i],SCR_HT-1-ly0[i],lx1[i],SCR_HT-1-ly1[i],c);
    if(mode&8) lcd.drawLine(SCR_WD-1-lx0[i],SCR_HT-1-ly0[i],SCR_WD-1-lx1[i],SCR_HT-1-ly1[i],c);
    cc+=1;
  }
}

unsigned long tm,demoTime=10000;

void loop()
{
  //lcd.fillScreen();
  //tm = millis();
  //while(millis()-tm<demoTime) circles();

  lcd.fillScreen();
  numLines=16;
  tm = millis();
  while(millis()-tm<demoTime) animLines(1);

  lcd.fillScreen();
  numLines=6;
  tm = millis();
  while(millis()-tm<demoTime) animLines(3);

  lcd.fillScreen();
  numLines=4;
  tm = millis();
  while(millis()-tm<demoTime) animLines(15);
}

