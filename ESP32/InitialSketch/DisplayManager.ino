














/*********************************************************************
This is an example for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

This example is for a 128x64 size display using SPI to communicate
4 or 5 pins are required to interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Display SPI Pins
#define OLED_MOSI  13
#define OLED_CLK   14
#define OLED_DC    5
#define OLED_CS    4
#define OLED_RESET 21

Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

void displayInit(){
  display.begin(SSD1306_SWITCHCAPVCC);
  display.clearDisplay();
  display.display();
  display.setTextColor(WHITE);
}

void updateDisplay(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Initial");
  if(SymphonyConnection.isConnected()){
    display.fillCircle(display.width()/2, 3, 3, WHITE);
  }
  else {
    display.drawCircle(display.width()/2, 3, 3, WHITE);
  }
  display.display();
}
