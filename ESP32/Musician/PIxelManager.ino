#include <Adafruit_NeoPixel.h>
#define LED_PIN 12

static const char* PIXEL_LOG_TAG = "Pixel"; 

Ticker pixelTask;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, LED_PIN, NEO_GRB + NEO_KHZ800);

enum PixelState {
  booting = 0,
  connecting,
  updating,
  executing,
  indicating,
  alerting
};

PixelState pixelState = booting;
int indicationCount = 0;

void setConnecting(){
  pixelState = connecting;  
}

void setUpdating(){
  pixelState = updating;  
}

void setExecuting(){
  pixelState = executing;
  pixelTask.detach();
  pixelOn();  
}

void setIndicating(){
  indicationCount = 25;
  pixelState = indicating;  
  pixelTask.detach();
  pixelOn();
}

void setAlerting(){
  pixelState = alerting;  
  pixelTask.detach();
  pixelOn();
}

int getOnTime(){
  switch (pixelState) {
    case booting:
      return 10;
    case connecting:
      return 10;
    case updating:
      return 10;
    case executing:
      return 5;
    case indicating:
      return 50;
    case alerting:
      return 100;
    default:
      return 1000;
  }
}

int getOffTime(){
  switch (pixelState) {
    case booting:
      return 1000;
    case connecting:
      return 1000;
    case updating:
      return 1000;
    case executing:
      return 5000;
    case indicating:
      return 100;
    case alerting:
      return 1000;
    default:
      return 1000;
  }
}

uint32_t getColor(){
  strip.setBrightness(25);
  
  switch (pixelState) {
    case booting:
      return strip.Color(255,0,0);
    case connecting:
      return strip.Color(0,255,0);
    case updating:
      return strip.Color(0,0,255);
    case executing:
      return strip.Color(255,255,0);
    case indicating:
      strip.setBrightness(200);
      return strip.Color(255,0,255);
    case alerting:
      strip.setBrightness(100);
      return strip.Color(0,255,255);
    default:
      return strip.Color(255,255,255);
  }

  return strip.Color(255,255,255);
}


void startPixel() {
  strip.begin();
  strip.setBrightness(50);
  strip.setPixelColor(0,strip.Color(50,10,10));
  strip.show();
  pixelOn();
}

/* ----- Onboard Neopixel ------------------------------------------ */

void pixelOn(){
  ESP_LOGD(PIXEL_LOG_TAG,"Pixel On");
  strip.setPixelColor(0, getColor());
  strip.show();
  strip.show();
  pixelTask.once_ms(getOnTime(), pixelOff);
}

void pixelOff(){
  ESP_LOGD(PIXEL_LOG_TAG,"Pixel Off");
  strip.setPixelColor(0, strip.Color(0,0,0));
  strip.show();
  strip.show();
  pixelTask.once_ms(getOffTime(), pixelOn);

  if( pixelState == alerting ) {
    setExecuting();    
  }
  if( indicationCount > 0 ){
    if(--indicationCount == 0){
      setExecuting();  
    }
  }
}
