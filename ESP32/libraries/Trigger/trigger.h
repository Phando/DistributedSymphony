/*
  Trigger.h - Library for for triggering time based actions
  Created by Joe Andolina, April 12, 2018.
  Released into the public domain.
*/

#ifndef TRIGGER_H
#define TRIGGER_H

#include "Arduino.h"

typedef void (* triggerFunction) ();

class Trigger {

protected:
  int triggerTime;

public:
  Trigger();
  Trigger(triggerFunction handler, int offset);
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

#endif
