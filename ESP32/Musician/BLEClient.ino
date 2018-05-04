/**
 * A BLE client example that is rich in capabilities.
 */
//#include "BLEClient.h"
#include "BLEDevice.h"

// The remote service we wish to connect to.
static BLEUUID  serviceUUID("78b940fc-131d-4cc5-961c-891153c0cce7");
static BLEUUID  ssidUUID("cd10d9cc-f8d5-468d-89a1-aac3149ab4f9");
static BLEUUID  passUUID("d8b8a3e7-2361-4b67-bf51-40b1efd340c0");
static BLEAddress serverOne = BLEAddress("b4:e6:2d:86:b5:ef");
static BLEAddress serverTwo = BLEAddress("b4:e6:2d:86:b5:ef");
  
void getBLECredentials() {
  Serial.begin(115200);
  delay(50);
  Serial.println("");
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");
  BLEClient*  client  = BLEDevice::createClient();
  
  if( client->connect(serverOne) ){
    Serial.println("Connected to BLE Server1");
    Serial.println(client->getValue(serviceUUID, ssidUUID).c_str());
    Serial.println(client->getValue(serviceUUID, passUUID).c_str());
  } 
  else if( client->connect(serverTwo) ){
    Serial.println("Connected to BLE Server1");
    Serial.println(client->getValue(serviceUUID, ssidUUID).c_str());
    Serial.println(client->getValue(serviceUUID, passUUID).c_str());
  } 
  else {
    Serial.println("Could not find any BLE servers");
  }
}
