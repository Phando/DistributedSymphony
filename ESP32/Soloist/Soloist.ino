#include <Arduino.h>
#include <ArduinoJson.h>
#include "BluetoothSerial.h"
#include <Preferences.h>

/** Build time */
const char compileDate[] = __DATE__ " " __TIME__;

/** Unique device name */
char apName[] = "ESP32-xxxxxxxxxxxx";
/** Selected network 
    true = use primary network
    false = use secondary network
*/
bool usePrimAP = true;
/** Flag if stored AP credentials are available */
bool hasCredentials = false;
/** Connection status */
volatile bool isConnected = false;
/** Connection change status */
bool connStatusChanged = false;

// SerialBT class
BluetoothSerial SerialBT;

/** Buffer for JSON string */
// MAx size is 51 bytes for frame: 
// {"ssidPrim":"","pwPrim":"","ssidSec":"","pwSec":""}
// + 4 x 32 bytes for 2 SSID's and 2 passwords
//StaticJsonBuffer<200> jsonBuffer;

/**
 * Create unique device name from MAC address
 **/
void createName() {
  uint8_t baseMac[6];
  // Get MAC address for WiFi station
  esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
  // Write unique name into apName
  sprintf(apName, "ESP32-%02X%02X%02X%02X%02X%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
  Serial.println("Name: " + String(apName));
}

/**
 * initBTSerial
 * Initialize Bluetooth Serial
 * Start BLE server and service advertising
 * @return <code>bool</code>
 *      true if success
 *      false if error occured
 */
bool initBTSerial() {
    Serial.println("Init BTSerial");
    
    if (!SerialBT.begin(apName)) {
      Serial.println("Failed to start BTSerial");
      return false;
    }
    Serial.println("BTSerial active. Device name: " + String(apName));
    return true;
}

/**
 * readBTSerial
 * read all data from BTSerial receive buffer
 * parse data for valid WiFi credentials
 */
void readBTSerial() {
  uint64_t startTimeOut = millis();
  String receivedData;
  int msgSize = 0;
  // Read RX buffer into String
  while (SerialBT.available() != 0) {
    receivedData += (char)SerialBT.read();
    msgSize++;
    // Check for timeout condition
    if ((millis()-startTimeOut) >= 5000) break;
  }
  SerialBT.flush();
  Serial.println("Received message " + receivedData + " over Bluetooth");

  // decode the message 
  int keyIndex = 0;
  for (int index = 0; index < receivedData.length(); index ++) {
    receivedData[index] = (char) receivedData[index] ^ (char) apName[keyIndex];
    keyIndex++;
    if (keyIndex >= strlen(apName)) keyIndex = 0;
  }

  Serial.println("Received message " + receivedData + " over Bluetooth");

  /** Json object for incoming data */
  //JsonObject& jsonIn = jsonBuffer.parseObject(receivedData);
  //if (jsonIn.success()) {
    /*
    if (jsonIn.containsKey("ssidPrim") &&
        jsonIn.containsKey("pwPrim") && 
        jsonIn.containsKey("ssidSec") &&
        jsonIn.containsKey("pwSec")) {
      ssidPrim = jsonIn["ssidPrim"].as<String>();
      pwPrim = jsonIn["pwPrim"].as<String>();
      ssidSec = jsonIn["ssidSec"].as<String>();
      pwSec = jsonIn["pwSec"].as<String>();

      Preferences preferences;
      preferences.begin("WiFiCred", false);
      preferences.putString("ssidPrim", ssidPrim);
      preferences.putString("ssidSec", ssidSec);
      preferences.putString("pwPrim", pwPrim);
      preferences.putString("pwSec", pwSec);
      preferences.putBool("valid", true);
      preferences.end();

      Serial.println("Received over bluetooth:");
      Serial.println("primary SSID: "+ssidPrim+" password: "+pwPrim);
      Serial.println("secondary SSID: "+ssidSec+" password: "+pwSec);
      */
      connStatusChanged = true;
      hasCredentials = true;
    //} 
  //} else {
  //  Serial.println("Received invalid JSON");
  //}
  //jsonBuffer.clear();
}


void setup() {
  // Create unique device name
  createName();

  // Initialize Serial port
  Serial.begin(115200);
  // Send some device info
  Serial.print("Build: ");
  Serial.println(compileDate);

  Preferences preferences;
  preferences.begin("WiFiCred", false);
  bool hasPref = preferences.getBool("valid", false);
  if (hasPref) {
    Serial.println("Has preferences");
  } else {
    Serial.println("No preferences, need send data over BLE");
  }
  preferences.end();

  // Start BTSerial
  initBTSerial();
}

void loop() {
  if (SerialBT.available() != 0) {
    // Get and parse received data
    readBTSerial();
  }
  Serial.print(".");
  delay(500);
} 

