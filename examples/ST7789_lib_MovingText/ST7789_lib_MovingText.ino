// ST7789 library example
// (C)2019-24 Pawel A. Hernik

// requires RRE Font library:
// https://github.com/cbm80amiga/RREFont

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

#include "RREFont.h"
#include "rre_chicago_20x24.h"

RREFont font;

// needed for RREFont library initialization, define your fillRect
void customRect(int x, int y, int w, int h, int c) { return lcd.fillRect(x, y, w, h, c); }

void setup() 
{
  Serial.begin(9600);
  lcd.init();
  lcd.fillScreen(BLACK);

  font.init(customRect, SCR_WD, SCR_HT); // custom fillRect function and screen width and height values
  font.setFont(&rre_chicago_20x24);
  int i;
  for(i=0;i<10;i++) {
    font.setColor(RGBto565(i*25,i*25,i*25));
    font.printStr(30+i,20+i,"Hello");
  }
  font.setColor(WHITE);
  font.printStr(30+i,20+i,"Hello");

  for(i=0;i<10;i++) {
    font.setColor(lcd.rgbWheel(0+i*8));
    font.printStr(25+i,60+i,"World");
  }
  font.setColor(WHITE);
  font.printStr(25+i,60+i,"World");
  delay(4000);
  lcd.fillScreen();
}

#define MAX_TXT 32
byte tx[MAX_TXT];
byte ty[MAX_TXT];
byte cur=0;
int numTxt=16;
int x=0,y=0;
int dx=6,dy=5;
int i,ii,cc=0;

void loop()
{
  x+=dx;
  y+=dy;
  if(x>SCR_WD-20) { dx=-dx; x=SCR_WD-20; }
  if(x<1)   { dx=-dx; x=0; }
  if(y>SCR_HT-20) { dy=-dy; y=SCR_HT-20; }
  if(y<1)   { dy=-dy; y=0; }
  int i=cur;
  //font.setColor(BLACK);
  //font.printStr(tx[i],ty[i],"Hello!");
  tx[cur]=x;
  ty[cur]=y;
  if(++cur>=numTxt) cur=0;
  for(i=0;i<numTxt;i++) {
    ii=i+cur;
    if(ii>=numTxt) ii-=numTxt;
    font.setColor(RGBto565(i*15,i*5,0));
    if(i==numTxt-1) font.setColor(WHITE);
    font.printStr(tx[ii],ty[ii],"Hello!");
    cc++;
  }
  delay(20);
}

