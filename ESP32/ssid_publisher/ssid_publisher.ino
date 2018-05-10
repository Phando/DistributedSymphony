/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
*/
#include <BLEDevice.h>
#include <BLEServer.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "78b940fc-131d-4cc5-961c-891153c0cce7"
#define SSID_UUID           "cd10d9cc-f8d5-468d-89a1-aac3149ab4f9"
#define PASS_UUID           "d8b8a3e7-2361-4b67-bf51-40b1efd340c0"

//#define SERVICE_SSID        "Rachel's Network"
//#define SERVICE_PASS        "photoshop!"

#define SERVICE_SSID        "Verizon-MiFi6620L-D537"
#define SERVICE_PASS        "a71745e9"

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("Starting BLE work!");
  BLEDevice::init("SymphonyConductor");
  
  Serial.print("BLE ADDRESS");
  Serial.println(BLEDevice::getAddress().toString().c_str());
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
  Serial.println("Characteristic defined! Now you can read it!");
}

void loop() {
  Serial.println("Characteristic Server Alive.");
  delay(10000);
}
