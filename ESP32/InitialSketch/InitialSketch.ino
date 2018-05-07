#include "OTAManager.h"

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("");
  Serial.println("Executing initial software:");
  Serial.println("Acuiring WiFi credentials and latest sketch.");
  OTAManager::getInstance().clearPreferences();
  OTAManager::getInstance().start();
}

void loop() {
  Serial.println("Initial software loop.");
  vTaskDelay(portMAX_DELAY);
}
