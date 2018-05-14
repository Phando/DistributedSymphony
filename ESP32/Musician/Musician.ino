#include <QueueArray.h>
#include <esp_log.h>

#include <SymphonyConnection.h>
#include <OTAManager.h>
#include "config.h"

#define BUT_PIN 22
#define NOTE_PIN 26
#define MIC_PIN 23
#define ENABLED 0 
#define DISABLED 1

static const char* LOG_TAG = "Musician";

SymphonyConnection connection;

QueueArray <int> notes;
Ticker gateCloseTask, gateOpenTask, reportTask;

bool dropTest = false;
bool shouldConnect = false;

// Interrupts and debouncing
unsigned long tempo = 300;
unsigned long bounceDelay = 150;
unsigned long lastButtonTime = 0;  // the last time the output pin was toggled
unsigned long lastImpactTime = 0;
unsigned long lastTouch1Time = 0;
unsigned long lastTouch2Time = 0;  


int dropMin, dropMax;
unsigned long dropTime = 0; 
unsigned long deviation = 0; 

unsigned long scheduleTime = 0;
unsigned long startTime = 0;

unsigned long gateTime = 100;

void setup() {
  ESP_LOGI(LOG_TAG,"Setup");
  Serial.begin(115200);
  delay(100);
  Serial.print("");
  
  uint64_t chipHex=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
  String chipId = chipIdToString(chipHex).substring(10);
  Serial.println("ESP32 Chip ID = " + chipId);//print High 2 bytes
  
  OTAManager::getInstance().clearPreferences();
  OTAManager::getInstance().start();
  
  pinMode(NOTE_PIN, OUTPUT);
  pinMode(MIC_PIN, INPUT_PULLUP);
  notes.setPrinter (Serial);

  // Setting up interrupts
  attachInterrupt(digitalPinToInterrupt(BUT_PIN), handleButton, RISING);
  attachInterrupt(digitalPinToInterrupt(MIC_PIN), handleImpact, RISING);
  touchAttachInterrupt(T2, gotTouch1, (10+touchRead(T2))/2);
  touchAttachInterrupt(T3, gotTouch2, (10+touchRead(T3))/2);

  ESP_LOGI(LOG_TAG,"Connecting to Symphony");
  //connection.init();
  connection.onMessage(handleMessage);
  
  //if (connection.connect()) {
  //  ESP_LOGI(LOG_TAG,"Successfully established Symphony Connection");
  //} else {
  //  ESP_LOGI(LOG_TAG,"Failed to establish Symphony Connection");
  //}
}

String chipIdToString(uint64_t input) {
  String result = "";
  uint8_t base = 10;

  do {
    char c = input % base;
    input /= base;

    if (c < 10)
      c +='0';
    else
      c += 'A' - 10;
    result = c + result;
  } while (input);
  return result;
}
/**
 * This callback will be in invoked for messages from server.
 */
void handleMessage(String message) {
  ESP_LOGI(LOG_TAG,"Message from server: %s", message);
  
  int len = message.length() + 1;
  char *tokens = new char[len];
  message.toCharArray(tokens, len);
  tokens = strtok(tokens, ":");
  
  if( String(tokens) == "CALIBRATE"){
    beginDropTest();
  }
  if( String(tokens) == "PLAY"){
    beginPlay(tokens);
  }
  if( String(tokens) == "DISABLE"){
    // TODO disable the user button
    // TODO disable "PLAY:CALIBRATE"
  }
  if( String(tokens) == "ENABLE"){
    // TODO enable the user button
    // TODO enable "PLAY:CALIBRATE"
  }
  
  if( String(tokens) == "ALIVE"){
    //doAlive();
  }
}

/* ----- Prepare for Test  ---------------------------------------------- */

void beginDropTest(){
  dropTest = true;
  dropMin = 50000;
  dropMax = 0;
  handleMessage("PLAY:1500:0:" + String(gateTime) + ":0:1:2:3:4");
}

/* ----- Prepare for Play  ---------------------------------------------- */

void beginPlay(char* tokens){
  gateCloseTask.detach();
  gateOpenTask.detach();
  startTime = 0;
  scheduleTime = 0;

  while( !notes.isEmpty() ){
    notes.pop();
  } 
  
  // TEMPO
  tokens = strtok(NULL, ":");
  if(tokens != NULL){
    ESP_LOGI(LOG_TAG,"Tempo: %s", tokens);
    tempo = atoi(tokens);
    bounceDelay = tempo / 2;
  }

  // SCHEDULE
  tokens = strtok(NULL, ":");
  if(tokens != NULL){
    ESP_LOGI(LOG_TAG,"Start Time: %s", tokens);
    scheduleTime = atoi(tokens);
  }

  // Gate Time
  tokens = strtok(NULL, ":");
  if(tokens != NULL){
    ESP_LOGI(LOG_TAG,"Gate Time: %s", tokens);
    gateTime = atoi(tokens);
  }

  // Notes
  tokens = strtok(NULL, ":"); 
  while( tokens != NULL ) {
    notes.push(atoi(tokens));
    tokens = strtok(NULL, ":"); 
  }
  
  // If there are notes play them
  if( notes.count() > 0 ) {
    int nextNote = notes.pop(); // Get the first note and start
    startTime = millis();
    
    if( scheduleTime == 0 ) {
      gateOpen();
    }
    else {
      scheduleTime += nextNote * tempo;
      scheduleTime -= startTime + deviation;
      gateOpenTask.once_ms(scheduleTime, gateOpen);
    }
  }
}

/* ------------------------------------------------------------------- */
/* ----- Interrupt Handlers ------------------------------------------ */
/* ------------------------------------------------------------------- */

/* ------------------------------------------------------------------- */
void gotTouch1(){
  if (lastTouch1Time > millis()) return;
  lastTouch1Time = 250 + millis();
  
  gateTime = _max(40, gateTime - 10);
  Serial.println("TOUCH1 : "+ String(gateTime));
}

/* ------------------------------------------------------------------- */
void gotTouch2(){
  if (lastTouch2Time > millis()) return;
  lastTouch2Time = 250 + millis();
  
  gateTime = _min(200, gateTime + 10);
  Serial.println("TOUCH2 : "+ String(gateTime));
}

/* ------------------------------------------------------------------- */
void handleButton() {
  if (lastButtonTime > millis()) return;
  ESP_LOGI(LOG_TAG,"Connected at: %d", connection.isConnected());
  if(!connection.isConnected()){
    shouldConnect = true;
  }
  lastButtonTime = bounceDelay + millis();
  gateOpen();
}

/* ------------------------------------------------------------------- */
void handleImpact() {
  if(lastImpactTime > millis()) return; 
  
  lastImpactTime = bounceDelay + millis();
  ESP_LOGI(LOG_TAG,"Impact at: %d", millis() - dropTime);
  
  //doPixel();
  
  if( dropTest ) {
    dropTime = millis() - dropTime;
    dropMin = _min(dropMin, dropTime);
    dropMax = _max(dropMax, dropTime);  
    
    if( notes.isEmpty() ){
      dropTest = false;
      deviation = (dropMax - dropMin) / 2;
      reportTask.once_ms(100, sendReport);
    }
    else {
      int nextNote = notes.pop();
      gateOpenTask.once_ms(tempo, gateOpen);
    }
  }
}

/* ----- Gate Open  ---------------------------------------------- */

void gateOpen(){
  dropTime = millis();
  digitalWrite(NOTE_PIN, HIGH);
  gateCloseTask.once_ms(gateTime, gateClose);
  
  ESP_LOGI(LOG_TAG,"GateOpen Time: %d", dropTime);
  lastImpactTime = dropTime + 10; // This fixes instant interrupts
  
  if( !notes.isEmpty() && !dropTest ){
    int nextNote = notes.pop();
    gateOpenTask.once_ms((startTime-dropTime)+(nextNote*tempo), gateOpen);
  }
}

/* ----- Gate Close  ---------------------------------------------- */

void gateClose(){
  ESP_LOGI(LOG_TAG,"GateClose Time: %d", millis());
  digitalWrite(NOTE_PIN, LOW);
}

/* ----- Send Report  ---------------------------------------------- */

void sendReport(){
  ESP_LOGI(LOG_TAG,"Reporting deviation - %d : %d : %d", dropMin, dropMax, deviation);
  connection.sendMessage("SET:deviation=" + String(deviation));
}

/* ----- Main Loop  ---------------------------------------------- */

void loop() {
  connection.workConnection();
  if(shouldConnect){
    shouldConnect = false;
    connection.init();
  }
  //vTaskDelay(portMAX_DELAY);
}
