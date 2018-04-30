
#include <TriggerPair.h>
#include <QueueArray.h>
#include <Preferences.h>
#include "SymphonyConnection.h"

#define NOTE_PIN 26
#define MIC_PIN 23
#define ENABLED 0 
#define DISABLED 1

#define max(a,b) ((a)>(b)?(a):(b)) 
#define min(a,b) ((a)<(b)?(a):(b))

SymphonyConnection connection;

//const char* ssid     = "herewego";
//const char* password = "photoshop!";

const char* ssid     = "Verizon-MiFi6620L-D537";
const char* password = "a71745e9";

//Store the gate time on the chip
//Store the clockSkew on the chip
//store the SSID and PW

Preferences prefs;
QueueArray <int> notes;

int bounceDelay = 250;

TriggerPair gate;
Trigger dropTest;

unsigned long lastBounce = 0;  
bool shouldReport = false;

int dropMin, dropMax;
unsigned long dropTime = 0; 
unsigned long deviation = 0; 
unsigned long startTime = 0;

void setup() {
  prefs.begin("distributed", false);

  Serial.begin(115200);
  pinMode(NOTE_PIN, OUTPUT);
  pinMode(MIC_PIN, INPUT_PULLUP);
  notes.setPrinter (Serial);
  
  dropTest  = Trigger( finishTest,10000);
  gate      = TriggerPair( gateOpen, 500, gateClose, 50);
  attachInterrupt(digitalPinToInterrupt(MIC_PIN), handleImpact, RISING);
    
  // Connect to network.
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  Serial.println("");
  Serial.println("\nWiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  checkVersion();
  connection.onMessage(handleMessage);

  if (connection.connect()) {
    Serial.println("Successfull Symphony Connection");
  } else {
    Serial.println("Unsuccessful Symphony Connection");
  }
}

/**
 * This callback will be in invoked for messages from server.
 */
void handleMessage(String message) {
  Serial.println("Message from server: " + String(message));
  
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
    doAlive();
  }
}

/* ----- Prepare for Test  ---------------------------------------------- */

void beginDropTest(){
  finishTest();
  dropMin = 50000;
  dropMax = 0;
  dropTest. reschedule();
  handleMessage("PLAY:1500:0:" + String(gate.betaOffset()) + ":0:1:2:3:4");
}

/* ----- Prepare for Play  ---------------------------------------------- */

void beginPlay(char* tokens){
  finishTest();
  startTime = 0;
  int schedule = 0;

  // TEMPO
  tokens = strtok(NULL, ":");
  if(tokens != NULL){
    Serial.println(tokens);
    gate.alphaOffset(atoi(tokens));
    bounceDelay = gate.alphaOffset() / 2;
  }

  // SCHEDULE
  tokens = strtok(NULL, ":");
  if(tokens != NULL){
    Serial.println(tokens);
    schedule = atoi(tokens);
  }

  // Gate Time
  tokens = strtok(NULL, ":");
  if(tokens != NULL){
    Serial.println(tokens);
    gate.betaOffset(atoi(tokens));
  }

  // Notes
  tokens = strtok(NULL, ":"); 
  while( tokens != NULL ) {
    notes.push(atoi(tokens));
    tokens = strtok(NULL, ":"); 
  }
  
  // Only if there are notes
  if( notes.count() > 0 ) {
    int startOffset = notes.pop() * gate.alphaOffset();
    if( schedule == 0 ) {
      gate.execute();
    }
    else {
      //startOffset -= deviation;
      gate.scheduleAlphaAt(schedule + startOffset);  
    }
  }
  
}

/* ----- Impact Handler ------------------------------------------ */

void handleImpact() {
  if(lastBounce > millis()) return; 

  Serial.println(millis() - dropTime);
  lastBounce = millis() + bounceDelay;
  doPixel();
  
  if( dropTest.isActive() ) {
    dropTime = millis() - dropTime;
    dropMin = min(dropMin, dropTime);
    dropMax = max(dropMax, dropTime);  
    
    if( notes.isEmpty() ){
      dropTest.invalidate();
      finishTest();
      shouldReport = true;
    }
    else {
      int note = notes.pop();
      gate.scheduleAlphaAt(startTime + (note * gate.alphaOffset()));   
    }
  }
}

/* ----- Finish Test  ---------------------------------------------- */

void finishTest() {
  gate.invalidate();
  deviation = (dropMax - dropMin) / 2;
  
  while( !notes.isEmpty() ){
    notes.pop();
  } 
}

/* ----- Gate Functions  ---------------------------------------------- */

void gateOpen(){
  Serial.println("GateOpen");
  digitalWrite(NOTE_PIN, HIGH);
  dropTime = millis();
  gate.rescheduleBeta();
  lastBounce = dropTime + gate.betaOffset() + 10; // This fixes instant interrupts
  if(startTime == 0){
    startTime = dropTime;
  }
  
 if( !notes.isEmpty() && !dropTest.isActive() ){
    int note = notes.pop();
    gate.scheduleAlphaAt(startTime + (note * gate.alphaOffset()));
  }
}

void gateClose(){
  Serial.println("GateClose");
  digitalWrite(NOTE_PIN, LOW);
}

/* ----- Main Loop  ---------------------------------------------- */

void loop() {
  pixelLoop();
  dropTest.tick();
  gate.tick();
  
  if( shouldReport ){
    shouldReport = false;
    Serial.println("Reporting deviation - " + String(dropMin) + " : " + String(dropMax) + " : " + String(deviation));
    connection.sendMessage("SET:deviation=" + String(deviation));
  }
}
