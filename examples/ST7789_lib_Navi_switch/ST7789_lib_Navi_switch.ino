// ST7789 library example
// 5-way navi switch example
// (c) 2019 Pawel A. Hernik
// https://youtu.be/EO7pfz1KjJM 

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
#if (__STM32F1__) // bluepill
#define TFT_DC     PA1
#define TFT_RST    PA0
#define NAVI_UP    PB15
#define NAVI_DOWN  PB14
#define NAVI_LEFT  PB13
#define NAVI_RIGHT PB12
#define NAVI_MID   PB10
#define NAVI_SET   PB9
#define NAVI_RST   PB8
//#include <Arduino_ST7789_STM.h>
#else
#define NAVI_UP    2
#define NAVI_DOWN  3
#define NAVI_LEFT  4
#define NAVI_RIGHT 5
#define NAVI_MID   6
#define NAVI_SET   7
#define NAVI_RST   8
#include "ST7789_AVR.h"
#define TFT_DC   10
//#define TFT_CS    9  // with CS
//#define TFT_RST  -1  // with CS
#define TFT_CS  -1 // without CS
#define TFT_RST  9 // without CS
#endif

#define SCR_WD 240
#define SCR_HT 240
ST7789_AVR lcd = ST7789_AVR(TFT_DC, TFT_RST, TFT_CS);


void setup() 
{
  Serial.begin(9600);
  pinMode(NAVI_UP,    INPUT_PULLUP);
  pinMode(NAVI_DOWN,  INPUT_PULLUP);
  pinMode(NAVI_LEFT,  INPUT_PULLUP);
  pinMode(NAVI_RIGHT, INPUT_PULLUP);
  pinMode(NAVI_MID,   INPUT_PULLUP);
  pinMode(NAVI_SET,   INPUT_PULLUP);
  pinMode(NAVI_RST,   INPUT_PULLUP);
  lcd.init();
  lcd.fillScreen();
}

void loop() 
{
  int wd=240,ht=240,rec=50,trih=40,triw=50;
  int wd2=wd/2,ht2=ht/2,rec2=rec/2;

  uint16_t c = digitalRead(NAVI_UP) ? RED : GREEN;
  lcd.fillTriangle(wd2,0, wd2-triw,trih, wd2+50,trih, c);
  lcd.fillRect(wd2-rec2,trih, rec,rec, c);

  c = digitalRead(NAVI_DOWN) ? RED : GREEN;
  lcd.fillTriangle(wd2,ht-0, wd2-triw,ht-trih, wd2+triw,ht-trih, c);
  lcd.fillRect(wd2-rec2,ht-trih-rec, rec,rec, c);

  c = digitalRead(NAVI_LEFT) ? RED : GREEN;
  lcd.fillTriangle(0,ht2, trih,ht2-triw, trih,ht2+triw, c);
  lcd.fillRect(trih,ht2-rec2, rec,rec, c);

  c = digitalRead(NAVI_RIGHT) ? RED : GREEN;
  lcd.fillTriangle(wd-0,ht2, wd-trih,ht2-triw, wd-triw,ht2+triw, c);
  lcd.fillRect(wd-trih-rec,ht2-rec2, rec,rec, c);

  c = digitalRead(NAVI_MID) ? RED : GREEN;
  lcd.fillRect(wd2-rec2,ht2-rec2, rec,rec, c);

  c = digitalRead(NAVI_SET) ? RED : GREEN;
  lcd.fillRect(0,ht-20, 40,20, c);

  c = digitalRead(NAVI_RST) ? RED : GREEN;
  lcd.fillRect(wd-40,ht-20, 40,20, c);
}

