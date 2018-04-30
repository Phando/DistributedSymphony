#include <Adafruit_NeoPixel.h>
#define LED_PIN 12

int blinkCount; 
TriggerPair alive, pixel;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, LED_PIN, NEO_GRB + NEO_KHZ800);

void pixelSetup(){
  strip.begin();
  strip.setBrightness(50);
  strip.setPixelColor(0,strip.Color(50,10,10));
  strip.show();
  strip.show();

  alive     = TriggerPair( aliveOn, 5000, aliveOff, 5);
  pixel     = TriggerPair( pixelOn, 100, pixelOff, 50);
  alive.execute();
}

/* ----- Onboard Neopixel ------------------------------------------ */

void pixelOn(){
  Serial.println("PixelOn");
  strip.setBrightness(100);
  strip.setPixelColor(0, strip.Color(20,200,20));
  strip.show();
  strip.show();
  pixel.rescheduleBeta();
}

void pixelOff(){
  Serial.println("PixelOff");
  aliveOff();
}

/* ----- Onboard Neopixel  ---------------------------------------------- */

void aliveOn(){
  if( pixel.isActive() ){
      return;
  }
  
  if(blinkCount > 0){
    blinkCount--;
    strip.setBrightness(150);
    strip.setPixelColor(0, strip.Color(150,50,50));
  }
  else {
    strip.setBrightness(10);
    strip.setPixelColor(0, strip.Color(23,152,193));
    alive.alphaOffset(5000);
    alive.betaOffset(5);   
  }
  
  strip.show();
  strip.show();
  alive.rescheduleBeta();
}

void aliveOff(){
  strip.setPixelColor(0, 0, 0, 0, 0);
  strip.show();
  strip.show();
  alive.rescheduleAlpha();
}

void doAlive(){
  blinkCount = 20;
  pixel.invalidate();
  alive.invalidate();
  alive.alphaOffset(100);
  alive.betaOffset(100);
  alive.execute();
}

void doPixel(){
  pixel.execute();
}

void pixelLoop(){
  alive.tick();
  pixel.tick();
}
