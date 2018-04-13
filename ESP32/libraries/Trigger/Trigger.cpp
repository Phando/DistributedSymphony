/*
  Morse.cpp - Library for flashing Morse code.
  Created by David A. Mellis, November 2, 2007.
  Released into the public domain.
*/

#include "Trigger.h"

Trigger::Trigger(){
  triggerTime = 0;
  offset = 0;
}

Trigger::Trigger(triggerFunction handler, int offset){
  triggerTime = 0;
  this->handler = handler;
  this->offset = offset;
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
