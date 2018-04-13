#include "TriggerPair.h"

TriggerPair::TriggerPair(){
}

TriggerPair::TriggerPair(triggerFunction AlphaHandler, int AlphaOffset, triggerFunction BetaHandler, int BetaOffset){
  alpha = Trigger(AlphaHandler,AlphaOffset);
  beta = Trigger(BetaHandler,BetaOffset);
}

TriggerPair::~TriggerPair(){
}

void TriggerPair::execute(){
  alpha.execute();
}

bool TriggerPair::isActive(){
  return alpha.isActive() || beta.isActive();
}

void TriggerPair::invalidate(){
  alpha.invalidate();
  beta.invalidate();
}

void TriggerPair::tick(){
  alpha.tick();
  beta.tick();
}

int TriggerPair::alphaOffset(){ return alpha.offset; }
void TriggerPair::alphaOffset(int offset){ alpha.offset = offset; }

int TriggerPair::betaOffset(){ return beta.offset; }
void TriggerPair::betaOffset(int offset){ beta.offset = offset; }

void TriggerPair::rescheduleAlpha(){
  alpha.reschedule();
}

void TriggerPair::rescheduleBeta(){
  beta.reschedule();
}

void TriggerPair::scheduleAlphaAt(int schedule){
  alpha.scheduleAt(schedule);
}

void TriggerPair::scheduleBetaAt(int schedule){
  beta.scheduleAt(schedule);
}

void TriggerPair::scheduleAlphaIn(int schedule){
  alpha.scheduleIn(schedule);
}

void TriggerPair::scheduleBetaIn(int schedule){
  beta.scheduleIn(schedule);
}

