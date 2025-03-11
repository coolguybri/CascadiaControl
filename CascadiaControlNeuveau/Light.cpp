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
  l->blinkTime = offset;
  l->blinkState = false;
  l->blinkIntervalOn = BLINK_INTERVAL_ON;
  l->blinkIntervalOff = BLINK_INTERVAL_OFF;

  bclogger("light_setup: pin=%d, state=%d, offset=%d, bstate=%b", 
    l->pin, l->state, l->blinkTime, l->blinkState);
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
void light_toggle_onoff(Light *l) {
   switch (l->state) {
    case LightState::Off:
      l->state = LightState::On;
      break;
      
    case LightState::On: 
      l->state = LightState::Off;
      break;
    
    default:
      bclogger("light_toggle_onoff: pin=%d, WIERD STATE CHANGE from %d", l->pin, l->state);
      l->state = LightState::Off;
      return;
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
      if (updateTime > l->blinkTime) {
          l->blinkState = !l->blinkState;
          l->blinkTime = l->blinkTime + (l->blinkState ? l->blinkIntervalOn : l->blinkIntervalOff);
      }
      break;
  }

  //bclogger("light_loop: pin=%d, state=%d, bstate=%b", l->pin, l->state, l->blinkState);
  
  // write out current state.
  digitalWrite(l->pin, l->blinkState);
}
