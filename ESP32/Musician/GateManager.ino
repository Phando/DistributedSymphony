
Ticker gateCloseTask, gateOpenTask, testTimeoutTask;

#define EPOCH_VALUE 1525910400 // Br the time a 32bit number
#define TEST_TIMEOUT 5000 // max drop time, in milliseconds.

bool lastDropDidTimeout = false;

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
  lastDropDidTimeout = false;
  testTimeoutTask.detach();
  average = 0;
  dropMax = 0;
  dropMin = 50000;
  testTimeoutTask.once_ms(TEST_TIMEOUT, timeoutDropTest);
  handleMessage("PLAY:1000:0:0:1:2:3:4");
}

void timeoutDropTest() {
  if (!dropTest) {
    return;
  }
  dropTest = false;
  lastDropDidTimeout = true;
  ESP_LOGI(LOG_TAG, "Timeout during drop test");
  updateDisplay();
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
      scheduleTime -= now + average;
      startTime += scheduleTime;
      scheduleTime += nextNote * tempo;
      gateOpenTask.once_ms(scheduleTime, gateOpen);
    }
  }
}

/* ------------------------------------------------------------------- */
void handleTouch1(){
  if (lastTouch1Time > millis() || shouldIgnore()) return;
  lastTouch1Time = 250 + millis();
  
  gateTime = _max(40, gateTime - 10);
  reportTask.once_ms(10, persistGate);
}

/* ------------------------------------------------------------------- */
void handleTouch2(){
  if (lastTouch2Time > millis() || shouldIgnore()) return;
  lastTouch2Time = 250 + millis();

  gateTime = _min(200, gateTime + 10);
  reportTask.once_ms(10, persistGate);
}

/* ------------------------------------------------------------------- */
void handleButton() {
  if (lastButtonTime > millis() || shouldIgnore() || dropTest) return;
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
    testTimeoutTask.detach();
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
      testTimeoutTask.once_ms(TEST_TIMEOUT, timeoutDropTest);
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

void persistGate(){
  ESP_LOGI(LOG_TAG,"Gate : %d", gateTime);
  SymphonyConnection.setParameter("gate", String(gateTime));
  updateDisplay();
}

/* ----- Send Report  ---------------------------------------------- */

void sendReport(){
  ESP_LOGI(LOG_TAG,"Reporting - min:%d  max:%d  dev:%d dt:%d", dropMin, dropMax, deviation, average );
  updateDisplay();
  SymphonyConnection.sendMessage("SET:dropTime=" + String(average));
  delay(500);
  SymphonyConnection.sendMessage("SET:deviation=" + String(deviation));
}

