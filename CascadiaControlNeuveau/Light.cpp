#include "arduino.h"
#include "Light.h"
#include "Logger.h"

#define BLINK_INTERVAL_ON 2000
#define BLINK_INTERVAL_OFF 250

/*
 * 
 */
void light_setup(Light *l, int pin, LightState state, int offset) {
  l->pin = pin;
  pinMode(l->pin, OUTPUT);

  l->state = state;
  l->blinkOffset = offset;
  l->blinkIntervalOn = BLINK_INTERVAL_ON;
  l->blinkIntervalOff = BLINK_INTERVAL_OFF;

  unsigned long now = millis();
  l->blinkState = false;
  l->blinkTimeNext = now + l->blinkOffset;
 
  bclogger("light_setup: pin=%d, state=%d, offset=%d, nextblink=%lu", 
    l->pin, l->state, l->blinkOffset, l->blinkTimeNext);
}


/*
 * 
 */
void light_update_state(Light *l, LightState state) {
   l->state = state;
   
   bclogger("light_update_state: pin=%d, state=%d", l->pin, l->state);
}

/*
 * 
 */
void light_update_blink(Light *l, unsigned long startTime, int offset, int durationOn, int durationOff) {
   l->blinkOffset = offset;
   
   l->blinkTimeNext = startTime + l->blinkOffset;
   l->blinkIntervalOn = durationOn;
   l->blinkIntervalOff = durationOff;
  
   bclogger("light_update_blink: pin=%d, state=%d, offset=%d, startTime=%lu, nextblink=%lu, duration=%d/%d", 
    l->pin, l->state, l->blinkOffset, startTime, l->blinkTimeNext, l->blinkIntervalOn, l->blinkIntervalOff);
}


/*
 *
 */
void light_toggle_onoff(Light *l) {
   switch (l->state) {
    case LightState::Off:
      l->state = LightState::On;
      break;
      
    case LightState::On: 
      l->state = LightState::Off;
      break;
    
    default:
      bclogger("light_toggle_onoff: pin=%d, jumping to off from %d", l->pin, l->state);
      l->state = LightState::Off;
      break;
   }
   
   bclogger("light_toggle_onoff: pin=%d, state=%d", l->pin, l->state);
}


/*
 * 
 */
boolean light_ison(Light *l) {
  return l->blinkState;
}


/*
 * 
 */
String light_get_state_name(Light *l) {
  switch (l->state) {
    case LightState::Off:
      return "off";
      
    case LightState::On: 
     return "on";

    case LightState::UniformBlink:
      return "blink";
    
    default:
      bclogger("light_toggle_onoff: pin=%d, ILLEGAL STATE CHANGE", l->pin);
      return;
   }
}


/*
 * 
 */
void light_loop(Light *l, unsigned long updateTime) {
  // update state based on time progressing.
  switch (l->state) {
    case LightState::Off:
      l->blinkState = false;
      break;
      
    case LightState::On: 
      l->blinkState = true;
      break;
    
    case LightState::UniformBlink:
      if (updateTime >= l->blinkTimeNext) {
          unsigned long thisTime = l->blinkTimeNext;
          
          int nextDuration = l->blinkState ? l->blinkIntervalOn : l->blinkIntervalOff;
          l->blinkState = !l->blinkState;
          l->blinkTimeNext = l->blinkTimeNext + nextDuration;
          
          //bclogger("light_loop: transition: pin=%d, state=%d, next=%d, update=%d", 
           // l->pin, l->state, l->blinkTimeNext, updateTime);

          bclogger("light_loop: pin=%d, state=%d, offset=%d, nextDur=%d, thisblink=%lu, nextblink=%lu, update=%lu", 
            l->pin, l->state, l->blinkOffset, nextDuration, thisTime, l->blinkTimeNext, updateTime);
      }
      break;
  }

  // Write out current state to the led.
  digitalWrite(l->pin, l->blinkState);
}
