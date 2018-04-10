#include <QueueArray.h>
#include "SymphonyConnection.h"
#include "Trigger.h"

#define LED_PIN 2
#define NOTE_PIN 25
#define MIC_PIN 15
#define ENABLED 0
#define DISABLED 1

#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))

const char* ssid     = "herewego";
const char* password = "photoshop!";

//const char* ssid     = "Verizon-MiFi6620L-D537";
//const char* password = "a71745e9";

SymphonyConnection connection;
QueueArray <int> notes;

typedef struct color
  {
    int r;
    int g;
    int b;
  };

int bounceDelay = 250;

color ledColor;
TriggerPair alive, gate, flash, pixel;
Trigger dropTest;

unsigned long lastBounce = 0;  

int blinkCount, dropMin, dropMax;
unsigned long dropTime = 0; 
unsigned long deviation = 0; 
unsigned long startTime = 0;

void setup() {
  Serial.begin(115200);
  pinMode(NOTE_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(MIC_PIN, INPUT_PULLUP);

  Serial.begin(115200);
  Serial.println();
  
  notes.setPrinter (Serial);
  attachInterrupt(digitalPinToInterrupt(MIC_PIN), handleImpact, RISING);
    
  // Connect to network.
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  ledColor.r = 50;
  ledColor.g = 50;
  ledColor.b = 50;

  dropTest.offset = 10000;
  dropTest.handler = finishTest;

  alive.on.offset = 5;
  alive.on.handler = aliveOn;
  alive.off.offset = 5000;
  alive.off.handler = aliveOff;
  
  gate.on.offset = 500; // Tempo
  gate.on.handler = gateOpen;
  gate.off.offset = 50; // GateTime
  gate.off.handler = gateClose;

  pixel.on.offset = 50;
  pixel.on.handler = pixelOn;
  pixel.off.offset = 50;
  pixel.off.handler = pixelOff;
  
//  handleMessage("DROPTEST");
//  Serial.println("");
//  Serial.println("\nWiFi connected");
//  Serial.println("IP address: ");
//  Serial.println(WiFi.localIP());
  connection.onMessage(handleMessage);

  if (connection.connect()) {
    Serial.println("Successfully Symphony Connection");
  } else {
    Serial.println("Unsuccessful Symphony Connection");
  }
  
  alive.execute();
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
  
  if( String(tokens) == "DROPTEST"){
    beginDropTest();
  }
  if( String(tokens) == "PLAYI"){
    beginPlay(tokens);
  }
  if( String(tokens) == "DISABLE"){
    // TODO disable the user button
  }
  if( String(tokens) == "ENABLE"){
    // TODO enable the user button
  }
  
  if( String(tokens) == "BLINK"){
    blinkCount = 10;
    alive.invalidate();
    alive.on.offset = 100;
    alive.off.offset = 100;
    alive.execute();
  }
}

/* ----- Prepare for Test  ---------------------------------------------- */

void beginDropTest(){
  finishTest();
  dropMin = 50000;
  dropMax = 0;
  dropTest.reschedule();
  handleMessage("PLAYI:1000:0:" + String(gate.off.offset) + ":0:1:2:3:4");
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
    gate.on.offset = atoi(tokens);
    bounceDelay = gate.on.offset/2;
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
    gate.off.offset = atoi(tokens);
  }

  // Notes
  tokens = strtok(NULL, ":"); 
  while( tokens != NULL ) {
    notes.push(atoi(tokens));
    tokens = strtok(NULL, ":"); 
  }
  
  // Only if there are notes
  if( notes.count() > 0 ) {
    int startOffset = notes.pop() * gate.on.offset;
    if( schedule == 0 ) {
      gate.execute();
    }
    else {
      //startOffset -= deviation;
      gate.on.scheduleAt(schedule + startOffset);  
    }
  }
  
}

/* ----- Impact Handler ------------------------------------------ */

void handleImpact() {
  if(lastBounce > millis()) return; 

  Serial.println(millis() - dropTime);
  lastBounce = millis() + bounceDelay;
  
  pixel.execute();
  
  if( dropTest.isActive() ) {
    dropTime = millis() - dropTime;
    dropMin = min(dropMin, dropTime);
    dropMax = max(dropMax, dropTime);  
    
    if( notes.isEmpty() ){
      dropTest.invalidate();
      finishTest();
    }
    else {
      int note = notes.pop();
      gate.on.scheduleAt(startTime + (note * gate.on.offset));   
    }
  }
}

/* ----- Finish Test  ---------------------------------------------- */

void finishTest() {
  gate.invalidate();
  deviation = (dropMax - dropMin) / 2;
  Serial.println(String(dropMin) + " : " + String(dropMax) + " : " + String(deviation));
  
  while( !notes.isEmpty() ){
    notes.pop();
  }
  // TODO : send score to server and screen;   
}

/* ----- Gate Functions  ---------------------------------------------- */

void gateOpen(){
  //Serial.println("GateOpen");
  digitalWrite(NOTE_PIN, HIGH);
  dropTime = millis();
  gate.off.reschedule();
  lastBounce = dropTime + gate.off.offset + 10; // This fixes instant interrupts
  if(startTime == 0){
    startTime = dropTime;
  }
  
 if( !notes.isEmpty() && !dropTest.isActive() ){
    int note = notes.pop();
    gate.on.scheduleAt(startTime + (note * gate.on.offset));
  }
}

void gateClose(){
  //Serial.println("GateClose");
  digitalWrite(NOTE_PIN, LOW);
}

/* ----- Onboard Neopixel ------------------------------------------ */

void pixelOn(){
  //Serial.println("PixelOn");
  pixel.off.reschedule();  
}

void pixelOff(){
  //Serial.println("PixelOff");
}

/* ----- Onboard LED  ---------------------------------------------- */

void aliveOn(){
  if(blinkCount-- == 0){
    alive.on.offset = 5;
    alive.off.offset = 5000;   
  }
  digitalWrite(LED_PIN, LOW);
  alive.off.reschedule();
}

void aliveOff(){
  digitalWrite(LED_PIN, HIGH);
  alive.on.reschedule();
}

void loop() {
  alive.tick();
  dropTest.tick();
  gate.tick();
  pixel.tick();
}
