#ifndef TRIGGER_H
#define TRIGGER_H

typedef void (* triggerFunction) ();

class Trigger {

protected:
  int triggerTime;

public:
  Trigger();
  ~Trigger();
  
  int offset;
  triggerFunction handler;

  void tick();
  void execute();
  void reschedule();
  void scheduleAt(int);
  void scheduleIn(int);
  void invalidate();
  bool isActive();
};

Trigger::Trigger(){
  triggerTime = 0;
  offset = 100;
}

Trigger::~Trigger(){
  handler = NULL;
}

void Trigger::tick(){
  if( isActive() && triggerTime < millis() ){
    invalidate();
    if( handler != NULL ){
      handler();
    }
  }
}

void Trigger::execute(){
  triggerTime = millis();
}

void Trigger::reschedule(){
  triggerTime = millis() + offset;
}

void Trigger::scheduleAt(int schedule){
  triggerTime = schedule;
}

void Trigger::scheduleIn(int schedule){
  triggerTime = millis() + schedule;
}

void Trigger::invalidate(){
  triggerTime = 0;
}

bool Trigger::isActive(){
  return triggerTime != 0;
}

class TriggerPair {
    public:
    Trigger on;
    Trigger off;

    TriggerPair();
    ~TriggerPair();
    
    void tick();
    void execute();
    void invalidate();
    bool isActive();
};

TriggerPair::TriggerPair(){
}

TriggerPair::~TriggerPair(){
}

void TriggerPair::tick(){
  on.tick();
  off.tick();
}

void TriggerPair::execute(){
  on.execute();
}

void TriggerPair::invalidate(){
  on.invalidate();
  off.invalidate();
}

bool TriggerPair::isActive(){
  return on.isActive() || off.isActive();
}

#endif
