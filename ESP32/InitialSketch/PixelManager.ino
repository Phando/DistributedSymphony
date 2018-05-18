#include <Adafruit_NeoPixel.h>
#define LED_PIN 12

static const char* PIXEL_LOG_TAG = "Pixel"; 

Ticker pixelTask;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, LED_PIN, NEO_GRB + NEO_KHZ800);

void startPixel() {
  strip.begin();
  strip.setBrightness(25);
  strip.setPixelColor(0,strip.Color(255,0,0));
  strip.show();
  






















}

/* ----- Onboard Neopixel ------------------------------------------ */

void pixelOn(){
  ESP_LOGD(PIXEL_LOG_TAG,"Pixel On");
  strip.setPixelColor(0,strip.Color(255,0,0));
  strip.show();
  strip.show();
  pixelTask.once_ms(15, pixelOff);
}

void pixelOff(){
  ESP_LOGD(PIXEL_LOG_TAG,"Pixel Off");
  strip.setPixelColor(0, strip.Color(0,0,0));
  strip.show();
  strip.show();
  pixelTask.once_ms(1000, pixelOn);
}
