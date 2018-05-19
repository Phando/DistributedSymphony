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

bool hasRemote = false;
String displayState = "empty";
String teamId = "";
String teamKey = "";
String state = "";
 
/**
 * This callback will be in invoked for state changes from the server.
 */
void handleStateChange(String message) {
  ESP_LOGI(LOG_TAG,"Server State: %s", message.c_str());
  
  if( message == "STOP"){
    displayState == "NONE";
    hasRemote = false;
  }
  if( message == "INTRO"){
  }
  if( message == "BUILD"){
    displayState = "URL";
  }
  if( message == "MANUAL"){
  }
  if( message == "FINALE"){
  }
  if( message == "WINNER"){
  }

  state = message;
  updateDisplay();
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
  
  if( String(tokens) == "TEST" && shouldRespond()){
    beginDropTest();
  }
  if( String(tokens) == "SYNC"){
    beginPlay(tokens);
  }
  if( String(tokens) == "PLAY" && shouldRespond()){
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
 
  hasRemote = true;
  updateDisplay();
}

/**
 * This callback will be in invoked when the server connection changes
 */
void handleChange(String message) {
  ESP_LOGI(LOG_TAG,"Change: %d", message);
  updateDisplay();
}

/**
 * This callback will be in invoked when the server connection changes
 */
void handleConnectionChange(bool message) {
  ESP_LOGI(LOG_TAG,"ConnectionChange: %d", message);
  updateDisplay();
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
  SymphonyConnection.onChange("key", handleChange);
  SymphonyConnection.onChange("team", handleChange);
  SymphonyConnection.onConnectionChange(handleConnectionChange);
  
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

/* ----- Utility functions to enforce state  ------------------------------------ */

bool shouldIgnore() {
  return state == "STOP" || state == "FINALE" || state == "WINNER";
}

bool shouldRespond() {
  return !shouldIgnore();
}

/* ----- Main Loop  ---------------------------------------------- */

void loop() {
  SymphonyConnection.workConnection();
}
