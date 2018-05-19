/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
*/
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <SPI.h>
#include <Wire.h>



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

#define SERVICE_UUID        "78b940fc-131d-4cc5-961c-891153c0cce7"
#define SSID_UUID           "cd10d9cc-f8d5-468d-89a1-aac3149ab4f9"
#define PASS_UUID           "d8b8a3e7-2361-4b67-bf51-40b1efd340c0"

//#define SERVICE_SSID        "Rachel's Network"
//#define SERVICE_PASS        "photoshop!"

#define SERVICE_SSID        "Verizon-MiFi6620L-D537"
#define SERVICE_PASS        "a71745e9"

#define CHAR_MAX 9
int charCount = 0;

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC);
  display.clearDisplay();
  display.display();
  display.setTextColor(WHITE);
  
  BLEDevice::init("SymphonyConductor");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *ssidCharacteristic = pService->createCharacteristic(
                                         SSID_UUID,
                                         BLECharacteristic::PROPERTY_READ
                                       );
  BLECharacteristic *passwordCharacteristic = pService->createCharacteristic(
                                         PASS_UUID,
                                         BLECharacteristic::PROPERTY_READ
                                       );
  ssidCharacteristic->setValue(SERVICE_SSID);
  passwordCharacteristic->setValue(SERVICE_PASS);
  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();
}

void updateDisplay(){
  String ssid = String(SERVICE_SSID);
  if(ssid.length() > 20){
    ssid = ssid.substring(0,17) + "...";  
  }
  
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.println("SSID Publisher\n\n" + ssid +"\n");
  display.println(BLEDevice::getAddress().toString().c_str());

  for(int i=0; i<CHAR_MAX; i++){
    display.drawCircle((i*14)+5, 58, 3, WHITE);  
  }
  for(int i=0; i<charCount; i++){
    display.fillCircle((i*14)+5, 58, 3, WHITE);  
  }
  display.display();
}

void loop() {
  if(charCount++ == CHAR_MAX-1){
    charCount = 0;  
  }
  updateDisplay();
  delay(2000);
}
