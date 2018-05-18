#include <QueueArray.h>
#include <esp_log.h>
#include <SymphonyConnection.h>
#include <Ticker.h>

#define BUT_PIN 22
#define NOTE_PIN1 26
#define MIC_PIN 23

const char* OHANA_PATTERN = ":0:2:3:4:6:10:12";
const char* LOG_TAG = "Musician";

QueueArray <int> notes;
Ticker reportTask;

unsigned long tempo = 300;  
unsigned long deviation = 0;
unsigned long gateTime = 100;

String displayState = "empty";
String teamId = "";
String teamKey = "";
String state = "";

/**
 * This callback will be in invoked for state changes from the server.
 */
void handleStateChange(String message) {
  ESP_LOGI(LOG_TAG,"Server State: %s", message.c_str());
  if( message == "Stop"){
  }
  if( message == "Intro"){
  }
  if( message == "Build"){
    displayState == "url";
  }
  if( message == "Manual"){
  }
  if( message == "Finale"){
  }
  if( message == "Winner"){
  }
  state = message;
}

/**
 * This callback will be in invoked for messages from the server.
 */
void handleMessage(String message) {
  ESP_LOGI(LOG_TAG,"Server Message: %s", message.c_str());
  
  int len = message.length() + 1;
  char *tokens = new char[len];
  message.toCharArray(tokens, len);
  tokens = strtok(tokens, ":");
  
  if( String(tokens) == "TEST"){
    beginDropTest();
  }
  if( String(tokens) == "SYNC"){
    beginPlay(tokens);
  }
  if( String(tokens) == "PLAY"){
    if(displayState != "ohana"){
      displayState = "empty";  
    }
    if(message.indexOf(OHANA_PATTERN)!=-1){
      displayState = "ohana"; 
    } 
    beginPlay(tokens);
  }
  if( String(tokens) == "ALIVE"){
    setIndicating();
  }
}

/**
 * This callback will be in invoked when the server connection changes
 */
void handleConnectionChange(String message) {
  ESP_LOGI(LOG_TAG,"ConnectionChange: %s", message.c_str());
}

/* ----- SETUP  ---------------------------------------------- */

void setup() {
  ESP_LOGI(LOG_TAG,"Setup");
  Serial.begin(115200);
  delay(100);
  Serial.print("");
  
  pinMode(NOTE_PIN1, OUTPUT);
  pinMode(MIC_PIN, INPUT_PULLUP);
  notes.setPrinter (Serial);
  
  displayInit();
  startPixel();
  
  // Setting up interrupts
  attachInterrupt(digitalPinToInterrupt(BUT_PIN), handleButton, RISING);
  attachInterrupt(digitalPinToInterrupt(MIC_PIN), handleImpact, RISING);
  touchAttachInterrupt(T2, handleTouch1, (10+touchRead(T2))/2);
  touchAttachInterrupt(T3, handleTouch2, (10+touchRead(T3))/2);

  ESP_LOGI(LOG_TAG,"Connecting to Symphony");
 
  SymphonyConnection.start();
  SymphonyConnection.onMessage(handleMessage);
  SymphonyConnection.onChange("state", handleStateChange);
  //SymphonyConnection.onConnectionStateChange(handleConnectionChange);
  
  getTeamId();  
  getTeamKey();  
  getState();
  updateDisplay();
}

/* ----- Get TeamId  ---------------------------------------------- */

void getTeamId(){
  teamId = SymphonyConnection.getParameter("team");
}

/* ----- Get Team Key ---------------------------------------------- */

void getTeamKey(){
  teamKey = SymphonyConnection.getParameter("key");
}

/* ----- Get Server State ---------------------------------------------- */

void getState(){
  state = SymphonyConnection.getParameter("state");
  handleStateChange(state);
}

/* ----- Main Loop  ---------------------------------------------- */

void loop() {
  SymphonyConnection.workConnection();
}
