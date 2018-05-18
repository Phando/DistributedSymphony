#include <esp_log.h>
#include <SymphonyConnection.h>

const char* LOG_TAG = "MAIN_SKETCH";

void handleConnectionChange(bool state) {
  updateDisplay();  
}

void setup() {
  ESP_LOGI(LOG_TAG,"Initial Sketch");
  displayInit();
  startPixel();
  pixelOn();
  
  ESP_LOGI(LOG_TAG,"Connecting to Symphony");
  SymphonyConnection.start();
  SymphonyConnection.clearPreferences();
  SymphonyConnection.onConnectionChange(handleConnectionChange);
  updateDisplay(); 
}

void loop() {
  SymphonyConnection.workConnection();
}
