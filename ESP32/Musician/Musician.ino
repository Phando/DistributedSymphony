#include <QueueArray.h>
#include <esp_log.h>
#include <SymphonyConnection.h>
#include <Ticker.h>

#define BUT_PIN 22
#define NOTE_PIN1 26
//#define NOTE_PIN2 27
#define MIC_PIN 23
#define ENABLED 0 
#define DISABLED 1

static const char* OHANA_PATTERN = ":0:2:3:4:6:10:12";
static const char* SURVEY_URL = "tinyurl.com/SFIS2018";
static const char* LOG_TAG = "Musician";
unsigned long MUSICIAN_WORK_PERIOD = 5000;
unsigned long workPeriod = 5000;

QueueArray <int> notes;
Ticker gateCloseTask, gateOpenTask, reportTask;

bool dropTest = false;
bool hasOhana = false;
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
unsigned long deviation = -1; 

unsigned long scheduleTime = 0;
unsigned long startTime = 0;
unsigned long gateTime = 100;

String teamId = "0000";
String teamKey = "Z2";
String state = "Build";

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
  
  if( String(tokens) == "CALIBRATE"){
    beginDropTest();
  }
  if( String(tokens) == "PLAY"){
    if(message.indexOf(OHANA_PATTERN)!=-1){
      hasOhana = true;  
    }
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
  updateDisplay();
  
  // Setting up interrupts
  attachInterrupt(digitalPinToInterrupt(BUT_PIN), handleButton, RISING);
  attachInterrupt(digitalPinToInterrupt(MIC_PIN), handleImpact, RISING);
  touchAttachInterrupt(T2, gotTouch1, (10+touchRead(T2))/2);
  touchAttachInterrupt(T3, gotTouch2, (10+touchRead(T3))/2);

  ESP_LOGI(LOG_TAG,"Connecting to Symphony");
 
  SymphonyConnection.start();
  SymphonyConnection.onMessage(handleMessage);
  SymphonyConnection.onChange("state", handleStateChange);

  ESP_LOGI(LOG_TAG,"State from storage: %s", SymphonyConnection.getParameter("state"));
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
    bounceDelay = tempo / 1.25;
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
  updateDisplay();
  ESP_LOGI(LOG_TAG,"TOUCH1 : %d", gateTime);
}

/* ------------------------------------------------------------------- */
void gotTouch2(){
  if (lastTouch2Time > millis()) return;
  lastTouch2Time = 250 + millis();
  
  gateTime = _min(200, gateTime + 10);
  updateDisplay();
  ESP_LOGI(LOG_TAG,"TOUCH2 : %d", gateTime);
}

/* ------------------------------------------------------------------- */
void handleButton() {
  if (lastButtonTime > millis()) return;
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
      reportTask.once_ms(100, sendDeviation);
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
  digitalWrite(NOTE_PIN1, HIGH);
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
  digitalWrite(NOTE_PIN1, LOW);
}

/* ----- Send Report  ---------------------------------------------- */

void sendDeviation(){
  ESP_LOGI(LOG_TAG,"Reporting deviation - %d : %d : %d", dropMin, dropMax, deviation);
  SymphonyConnection.sendMessage("SET:deviation=" + String(deviation));
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
  if( workPeriod < millis() ){
    workPeriod = MUSICIAN_WORK_PERIOD + millis();
    if( teamId == "0000"){ getTeamId();  }
    if( teamKey == "Z"){ getTeamKey();  }
    if( state == "Empty"){ getState();  }
    updateDisplay();
  }
}
