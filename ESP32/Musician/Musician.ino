#include <Adafruit_NeoPixel.h>
#include <TriggerPair.h>
#include <QueueArray.h>

#include "SymphonyConnection.h"

#define LED_PIN 12
#define NOTE_PIN 26
#define MIC_PIN 23
#define ENABLED 0 
#define DISABLED 1

#define max(a,b) ((a)>(b)?(a):(b)) 
#define min(a,b) ((a)<(b)?(a):(b))

const char* ssid     = "herewego";
const char* password = "photoshop!";

//const char* ssid     = "Verizon-MiFi6620L-D537";
//const char* password = "a71745e9";

//Store the gate time on the chip
//Store the clockSkew on the chip
//store the SSID and PW

SymphonyConnection connection;

QueueArray <int> notes;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, LED_PIN, NEO_GRB + NEO_KHZ800);

int bounceDelay = 250;

TriggerPair alive, gate, flash, pixel;
Trigger dropTest;

unsigned long lastBounce = 0;  

int blinkCount, dropMin, dropMax;
unsigned long dropTime = 0; 
unsigned long deviation = 0; 
unsigned long startTime = 0;

void setup() {
  strip.begin();
  strip.setBrightness(50);
  strip.setPixelColor(0,strip.Color(50,10,10));
  strip.show();
  strip.show();
  
  Serial.begin(115200);
  pinMode(NOTE_PIN, OUTPUT);
  pinMode(MIC_PIN, INPUT_PULLUP);
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

  dropTest  = Trigger( finishTest,10000);
  alive     = TriggerPair( aliveOn, 5000, aliveOff, 5);
  gate      = TriggerPair( gateOpen, 500, gateClose, 50);
  pixel     = TriggerPair( pixelOn, 100, pixelOff, 50);

  alive.execute();
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
  }
  if( String(tokens) == "ENABLE"){
    // TODO enable the user button
  }
  
  if( String(tokens) == "ALIVE"){
    blinkCount = 20;
    alive.invalidate();
    alive.alphaOffset(100);
    alive.betaOffset(100);
    alive.execute();
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
      gate.scheduleAlphaAt(startTime + (note * gate.alphaOffset()));   
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

/* ----- Onboard Neopixel ------------------------------------------ */

void pixelOn(){
  Serial.println("PixelOn");
  strip.setBrightness(100);
  strip.setPixelColor(0, strip.Color(20,200,20));
  strip.show();
  strip.show();
  pixel.rescheduleBeta();
}

void pixelOff(){
  Serial.println("PixelOff");
  aliveOff();
}

/* ----- Onboard LED  ---------------------------------------------- */

void aliveOn(){
  if( pixel.isActive() ){
      return;
  }
  
  if(blinkCount > 0){
    blinkCount--;
    strip.setBrightness(150);
    strip.setPixelColor(0, strip.Color(150,50,50));
  }
  else {
    strip.setBrightness(10);
    strip.setPixelColor(0, strip.Color(23,152,193));
    alive.alphaOffset(5000);
    alive.betaOffset(5);   
  }
  
  strip.show();
  strip.show();
  alive.rescheduleBeta();
}

void aliveOff(){
  strip.setPixelColor(0, 0, 0, 0, 0);
  strip.show();
  strip.show();
  alive.rescheduleAlpha();
}

void loop() {
  alive.tick();
  dropTest.tick();
  gate.tick();
  pixel.tick();
}
