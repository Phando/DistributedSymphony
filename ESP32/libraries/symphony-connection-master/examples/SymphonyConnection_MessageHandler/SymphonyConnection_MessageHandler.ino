#include "SymphonyConnection.h"

const char* ssid     = "SSID HERE";
const char* password = "PASSWORD HERE";

SymphonyConnection connection;

/**
 * This callback will be in invoked for messages from server.
 */
void logMessage(String message) {
  Serial.print("Message from server: ");
  Serial.println(message);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(100);

  Serial.println();
    // Connect to network.
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    connection.onMessage(logMessage);

    if (connection.connect()) {
      Serial.println("Successfully established Symphony Connection");
    } else {
      Serial.println("Failed to establish Symphony Connection");
    }
}

void loop() {
  // put your main code here, to run repeatedly:
}
