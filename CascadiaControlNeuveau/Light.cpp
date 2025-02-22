#include "arduino.h"
#include "Light.h"

#define BLINK_INTERVAL_ON 2000
#define BLINK_INTERVAL_OFF 250

/*
 * 
 */
void light_setup(Light *l, int pin, LightState state, int offset) {
  l->pin = pin;
  pinMode(l->pin, OUTPUT);

  l->state = state;
  l->blinkTime = offset;
  l->blinkState = false;
  l->blinkIntervalOn = BLINK_INTERVAL_ON;
  l->blinkIntervalOff = BLINK_INTERVAL_OFF;

  char logbuf[500];
  snprintf(logbuf, 500, "light_setup: pin=%d, state=%d, offset=%d, bstate=%b", 
    l->pin, l->state, l->blinkTime, l->blinkState);
  Serial.println(logbuf);
}


/*
 * 
 */
void light_update_state(Light *l, LightState state) {
   l->state = state;
   
   char logbuf[500];
   snprintf(logbuf, 500, "light_update_state: pin=%d, state=%d", l->pin, l->state);
   Serial.println(logbuf);
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
      char logbuf[500];
      snprintf(logbuf, 500, "light_toggle_onoff: pin=%d, ILLEGAL STATE CHANGE", l->pin);
      Serial.println(logbuf);
      return;
   }
   
   char logbuf[500];
   snprintf(logbuf, 500, "light_toggle_onoff: pin=%d, state=%d", l->pin, l->state);
   Serial.println(logbuf);
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
      if (updateTime > l->blinkTime) {
          l->blinkState = !l->blinkState;
          l->blinkTime = l->blinkTime + (l->blinkState ? l->blinkIntervalOn : l->blinkIntervalOff);
      }
      break;
  }

  char logbuf[500];
  snprintf(logbuf, 500, "light_loop: pin=%d, state=%d, bstate=%b", l->pin, l->state, l->blinkState);
  Serial.println(logbuf);
  
  // write out current state.
  digitalWrite(l->pin, l->blinkState);
}
