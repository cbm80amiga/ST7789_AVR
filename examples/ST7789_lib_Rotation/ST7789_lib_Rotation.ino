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
  Serial.begin(115200);
  lcd.init(SCR_WD,SCR_HT);
  lcd.setRotation(0);
}

void loop(void) 
{
  for(uint8_t rot = 0; rot < 4; rot++) {
    testText(rot);
    delay(2000);
  }
}

unsigned long testText(int rot)
{
  lcd.setRotation(rot);
  lcd.fillScreen(BLACK);
  lcd.setCursor(0, 0);
  lcd.setTextColor(BLUE);
  lcd.setTextSize(1);
  lcd.println("Hello World!");
  lcd.setTextColor(WHITE);
  lcd.print("Rotation = ");
  lcd.println(rot);
  lcd.setTextColor(YELLOW);
  lcd.setTextSize(2);
  lcd.println(1234.56);
  lcd.setTextColor(RED);
  lcd.setTextSize(3);
  lcd.println(0xDEAD, HEX);
  lcd.println();
  lcd.setTextColor(GREEN);
  lcd.setTextSize(4);
  lcd.println("Hello");
}


