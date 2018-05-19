
Ticker gateCloseTask, gateOpenTask;

#define EPOCH_VALUE 1525910400 // Br the time a 32bit number

bool dropTest = false;

unsigned long dropMin, dropMax;
unsigned long dropTime = 0; 
unsigned long average = -1;
unsigned long scheduleTime = 0;
unsigned long startTime = 0;

// Interrupts and debouncing
unsigned long bounceDelay = 150;
unsigned long lastButtonTime = 0;  // the last time the output pin was toggled
unsigned long lastImpactTime = 0;
unsigned long lastTouch1Time = 0;
unsigned long lastTouch2Time = 0;

/* ----- Prepare for Test  ---------------------------------------------- */

void beginDropTest(){
  dropTest = true;
  average = 0;
  dropMax = 0;
  dropMin = 50000;
  handleMessage("PLAY:1500:0:0:1:2:3:4");
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
      timeval tv = {0,0};
      gettimeofday(&tv, NULL);
      tv.tv_sec -= EPOCH_VALUE;
      unsigned long now = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
      Serial.println("Start: " + String(startTime));
      Serial.println("Schedule: " + String(scheduleTime));
      //scheduleTime -= now + deviation;
      scheduleTime -= now + average;
      startTime += scheduleTime;
      Serial.println("Start Adj: " + String(startTime));
      Serial.println("Schedule Adj1: " + String(scheduleTime));
      scheduleTime += nextNote * tempo;
      Serial.println("Schedule Adj2: " + String(scheduleTime));
      gateOpenTask.once_ms(scheduleTime, gateOpen);
    }
  }
}

/* ------------------------------------------------------------------- */
void handleTouch1(){
  if (lastTouch1Time > millis() || shouldIgnore()) return;
  lastTouch1Time = 250 + millis();
  
  gateTime = _max(40, gateTime - 10);
  updateDisplay();
  ESP_LOGI(LOG_TAG,"TOUCH1 : %d", gateTime);
}

/* ------------------------------------------------------------------- */
void handleTouch2(){
  if (lastTouch2Time > millis() || shouldIgnore()) return;
  lastTouch2Time = 250 + millis();
  
  gateTime = _min(200, gateTime + 10);
  updateDisplay();
  ESP_LOGI(LOG_TAG,"TOUCH2 : %d", gateTime);
}

/* ------------------------------------------------------------------- */
void handleButton() {
  if (lastButtonTime > millis() || shouldIgnore()) return;
  lastButtonTime = bounceDelay + millis();
  gateOpen();
}

/* ------------------------------------------------------------------- */
void handleImpact() {
  if(lastImpactTime > millis()) return; 
  
  dropTime = millis() - dropTime;
  lastImpactTime = millis() + bounceDelay;
  ESP_LOGI(LOG_TAG,"Impact at: %d", dropTime);
  setAlerting();

  if( dropTest ) {
    dropMin = _min(dropMin, dropTime);
    dropMax = _max(dropMax, dropTime);
    average += dropTime;
        
    if( notes.isEmpty() ){
      dropTest = false;
      average /= 5; 
      deviation = (dropMax - dropMin) / 2;
      ESP_LOGI(LOG_TAG,"Deviation: %d", deviation);
      reportTask.once_ms(100, sendReport);
    }
    else {
      notes.pop();
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
    gateOpenTask.once_ms((startTime-dropTime)+nextNote*tempo, gateOpen);
  }
}

/* ----- Gate Close  ---------------------------------------------- */

void gateClose(){
  ESP_LOGI(LOG_TAG,"GateClose Time: %d", millis());
  digitalWrite(NOTE_PIN1, LOW);
}

/* ----- Send Report  ---------------------------------------------- */

void sendReport(){
  ESP_LOGI(LOG_TAG,"Reporting - min:%d  max:%d  dev:%d dt:%d", dropMin, dropMax, deviation, average );
  updateDisplay();
  SymphonyConnection.sendMessage("SET:dropTime=" + String(average));
  delay(100);
  SymphonyConnection.sendMessage("SET:deviation=" + String(deviation));
}

