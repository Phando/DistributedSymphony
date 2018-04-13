/*
  Trigger.h - Library for for triggering time based actions
  Created by Joe Andolina, April 12, 2018.
  Released into the public domain.
*/

#ifndef TRIGGER_PAIR_H
#define TRIGGER_PAIR_H

#include "Arduino.h"
#include "Trigger.h"

class TriggerPair {
    protected:
      Trigger alpha;
      Trigger beta;
    public:
    
    TriggerPair();
    TriggerPair(Trigger a, Trigger b);
    TriggerPair(triggerFunction alphaHandler, int alphaOffset, triggerFunction betaHandler, int betaOffset);
    ~TriggerPair();
    
    void execute();
    void invalidate();
    bool isActive();
    void tick();

    void rescheduleAlpha();
    void rescheduleBeta();
  	void scheduleAlphaAt(int);
  	void scheduleBetaAt(int);
  	void scheduleAlphaIn(int);
  	void scheduleBetaIn(int);
    
    int alphaOffset();
    void alphaOffset(int);
    int betaOffset();
    void betaOffset(int);
};

#endif