#include <WiFi.h>
#include <Update.h>
#include <HTTPClient.h>
#include "config.h"

WiFiClient otaClient;

int contentLength = 0;
bool isValidContentType = false;

int port = 80; 
float targetVersion = 0.0;

// Utility to extract header value from headers
String getHeaderValue(String header, String headerName) {
  return header.substring(strlen(headerName.c_str()));
}

void checkVersion(){
  Serial.println("Checking sketch vsersion.");
  
  HTTPClient http;
  http.begin("http://" + OTA_HOST + OTA_VERSION);
  int httpCode = http.GET();                             
  
  if (httpCode > 0) { //Check for the returning code
      targetVersion = http.getString().toFloat();
      if (httpCode == HTTP_CODE_OK) {  
        float chipVersion = prefs.getFloat("sketchVersion", 0.0);
        Serial.println(String(chipVersion) + " : " + String(targetVersion));
        if ( chipVersion < targetVersion ){
           Serial.println("Updating sketch to version : " + String(targetVersion));
           execOTA();
        } else {
           Serial.println("Running with sketch version : " + String(targetVersion));
        }
      }
      else {
        Serial.println("Error getting "+ OTA_VERSION +" - "+ String(httpCode));
      }
    }
  else {
    Serial.println("Error on HTTP request");
  }
  http.end(); //Free the resources
}


// OTA Logic 
void execOTA() {
  Serial.println("Connecting to: " + OTA_HOST);
  // Connect to OTA Server
  if (otaClient.connect(OTA_HOST.c_str(), port)) {
    // Connection Succeed.
    // Fecthing the bin
    Serial.println("Fetching Bin: " + OTA_SKETCH);

    // Get the contents of the bin file
    otaClient.print(String("GET ") + OTA_SKETCH + " HTTP/1.1\r\n" +
                 "Host: " + OTA_HOST + "\r\n" +
                 "Cache-Control: no-cache\r\n" +
                 "Connection: close\r\n\r\n");

    // Check what is being sent
    //    Serial.print(String("GET ") + OTA_SKETCH + " HTTP/1.1\r\n" +
    //                 "Host: " + OTA_HOST + "\r\n" +
    //                 "Cache-Control: no-cache\r\n" +
    //                 "Connection: close\r\n\r\n");

    unsigned long timeout = millis();
    while (otaClient.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println("Client Timeout !");
        otaClient.stop();
        return;
      }
    }
    // Once the response is available,
    // check stuff

    /*
       Response Structure
        HTTP/1.1 200 OK
        x-amz-id-2: NVKxnU1aIQMmpGKhSwpCBh8y2JPbak18QLIfE+OiUDOos+7UftZKjtCFqrwsGOZRN5Zee0jpTd0=
        x-amz-request-id: 2D56B47560B764EC
        Date: Wed, 14 Jun 2017 03:33:59 GMT
        Last-Modified: Fri, 02 Jun 2017 14:50:11 GMT
        ETag: "d2afebbaaebc38cd669ce36727152af9"
        Accept-Ranges: bytes
        Content-Type: application/octet-stream
        Content-Length: 357280
        Server: AmazonS3
                                   
        {{BIN FILE CONTENTS}}

    */
    while (otaClient.available()) {
      // read line till /n
      String line = otaClient.readStringUntil('\n');
      // remove space, to check if the line is end of headers
      line.trim();

      // if the the line is empty, this is end of headers break the while and feed the remaining `client` to the Update.writeStream();
      if (!line.length()) {
        //headers ended
        break; // and get the OTA started
      }

      // Check if the HTTP Response is 200 else break and Exit Update
      if (line.startsWith("HTTP/1.1")) {
        if (line.indexOf("200") < 0) {
          Serial.println("Got a non 200 status code from server. Exiting OTA Update.");
          break;
        }
      }

      // extract headers here start with content length
      if (line.startsWith("Content-Length: ")) {
        contentLength = atoi((getHeaderValue(line, "Content-Length: ")).c_str());
        Serial.println("Got " + String(contentLength) + " bytes from server");
      }
      
      // Next, the content type
      if (line.startsWith("Content-Type: ")) {
        String contentType = getHeaderValue(line, "Content-Type: ");
        Serial.println("Got " + contentType + " payload.");
        if (contentType == "application/octet-stream") {
          isValidContentType = true;
        }
      }
    }
  } else {
    // Connect to OTA host failed
    // May be try?
    // Probably a choppy network?
    Serial.println("Connection to " + OTA_HOST + " failed. Please check your setup");
    // retry??
    // execOTA();
  }

  // Check what is the contentLength and if content type is `application/octet-stream`
  Serial.println("contentLength : " + String(contentLength) + ", isValidContentType : " + String(isValidContentType));

  // check contentLength and content type
  if (contentLength && isValidContentType) {
    // Check if there is enough to OTA Update
    bool canBegin = Update.begin(contentLength);

    // If yes, begin
    if (canBegin) {
      Serial.println("Begin OTA. This may take 2 - 5 mins to complete. Things might be quite for a while.. Patience!");
      // No activity would appear on the Serial monitor
      // So be patient. This may take 2 - 5mins to complete
      size_t written = Update.writeStream(otaClient);

      if (written == contentLength) {
        Serial.println("Written : " + String(written) + " successfully");
      } else {
        Serial.println("Written only : " + String(written) + "/" + String(contentLength) + ". Retry?" );
        // retry??
        // execOTA();
      }

      if (Update.end()) {
        Serial.println("OTA done!");
        if (Update.isFinished()) {
          Serial.println("Update successfully completed. Rebooting.");
          prefs.putFloat("sketchVersion", targetVersion);
          ESP.restart();
        } else {
          Serial.println("Update not finished? Something went wrong!");
        }
      } else {
        Serial.println("Error Occurred. Error #: " + String(Update.getError()));
      }
    } else {
      // not enough space to begin OTA
      // Understand the partitions and
      // space availability
      Serial.println("Not enough space to begin OTA");
      otaClient.flush();
    }
  } else {
    Serial.println("There was no content in the response");
    otaClient.flush();
  }
}
