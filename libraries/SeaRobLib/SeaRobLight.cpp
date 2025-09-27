#include "Arduino.h"
#include "SeaRobLight.h"
#include "SeaRobLogger.h"


#define BLINK_INTERVAL_ON 2000
#define BLINK_INTERVAL_OFF 250


/*
 */
SeaRobLight::SeaRobLight(int p, int offset): pin(p) {
  this->state = LightState::Off;
  this->loggingState = false;
  
  // this->loggingState = true;
  
  this->blinkOffset = offset;
  this->blinkIntervalOn = BLINK_INTERVAL_ON;
  this->blinkIntervalOff = BLINK_INTERVAL_OFF;
  this->litState = false;
  this->blinkTimeNext = 0;

  unsigned long now = millis();
  if (this->blinkOffset > 0) {
    this->blinkTimeNext = (now + this->blinkOffset);
  }

  pinMode(this->pin, OUTPUT);
 
  bclogger("SeaRobLight: pin=%d, state=%d, offset=%d, nextblink=%lu", 
    this->pin, this->state, this->blinkOffset, this->blinkTimeNext);
}


/*
 */
void SeaRobLight::SetDebugLogging(bool setter) {
  bool prev = this->loggingState;
  this->loggingState = setter;

  if (this->loggingState != prev) {
    bclogger("SeaRobLight: pin=%d, logging=%b", this->pin, this->loggingState);
  }
}


/*
 */
void SeaRobLight::UpdateState(LightState state) {
   this->state = state;

   if (this->loggingState) {
      bclogger("SeaRobLight::UpdateState: pin=%d, state=%d", this->pin, this->state);
   }
}


/*
 */
void SeaRobLight::UpdateBlinkConfig(unsigned long startTime, int offset, int durationOn, int durationOff) {
   this->blinkOffset = offset;
   this->blinkIntervalOn = durationOn;
   this->blinkIntervalOff = durationOff;
   this->blinkTimeNext = startTime + this->blinkOffset;

   if (this->loggingState) {
     bclogger("SeaRobLight::UpdateBlinkConfig: pin=%d, state=%d, offset=%d, startTime=%lu, nextblink=%lu, duration=%d/%d", 
        this->pin, this->state, this->blinkOffset, startTime, this->blinkTimeNext, this->blinkIntervalOn, this->blinkIntervalOff);
   }
}


/*
 */
void SeaRobLight::ToggleOnOff() {
   switch (this->state) {
    case LightState::Off:
      this->state = LightState::On;
      break;
      
    case LightState::On: 
      this->state = LightState::Off;
      break;
    
    default:
      if (this->loggingState) {
        bclogger("SeaRobLight::ToggleOnOff: pin=%d, jumping to off from %d", this->pin, this->state);
      }
      this->state = LightState::Off;
      break;
   }

   if (this->loggingState) {
      bclogger("SeaRobLight::ToggleOnOff: pin=%d, state=%d", this->pin, this->state);
   }
}


/* 
 */
boolean SeaRobLight::IsOn() {
  return this->litState;
}


/*
 */
String SeaRobLight::GetStateName() {
  switch (this->state) {
    case LightState::Off:
      return "off";
      
    case LightState::On: 
     return "on";

    case LightState::UniformBlink:
      return "blink";
    
    default:
      bclogger("SeaRobLight::GetStateName: pin=%d, ILLEGAL STATE CHANGE", this->pin);
      return "illegal-state";
   }
}


/*
 */
void SeaRobLight::ProcessLoop(unsigned long updateTime) {
  switch (this->state) {
    case LightState::Off:
      this->litState = false;
      break;
      
    case LightState::On: 
      this->litState = true;
      break;
    
    case LightState::UniformBlink:
      if (updateTime >= this->blinkTimeNext) {
          unsigned long thisTime = this->blinkTimeNext;
          
          int nextDuration = this->litState ? this->blinkIntervalOn : this->blinkIntervalOff;
          this->litState = !this->litState;
          this->blinkTimeNext = this->blinkTimeNext + nextDuration;
          
          if (this->loggingState) {
              bclogger("light_loop: pin=%d, state=%d, offset=%d, nextDur=%d, thisblink=%lu, nextblink=%lu, update=%lu", 
                this->pin, this->state, this->blinkOffset, nextDuration, thisTime, this->blinkTimeNext, updateTime);
          }
      }
      break;
  }

  // Write out current state to the led.
  digitalWrite(this->pin, this->litState);
}
