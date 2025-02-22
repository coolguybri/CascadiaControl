#include "arduino.h"
#include "MonorailSystem.h"

#define BLINK_OFFSET 100


/*
 * 
 */
void monorail_pole_setup(MonorailPole *pole, int startPin, int offsetYellow, int offsetOrange, LightState lightState) {
  light_setup(&pole->_lightYellow, startPin, lightState, offsetYellow);
  light_setup(&pole->_lightOrange, startPin + 1, lightState, offsetOrange);
}


/*
 * 
 */
void monorail_pole_loop(MonorailPole *pole, unsigned long updateTime) {
  light_loop(&pole->_lightYellow, updateTime);
  light_loop(&pole->_lightOrange, updateTime);
}


/*
 *
 */
void monorail_system_setup(MonorailSystem *monorail, int startPin) {
  monorail->_pinStart = startPin;
  monorail->_lightState = LightState::Off;
  monorail->_lightStateStartTime = 0;

  for (int i = 0 ; i < MONORAIL_POLE_COUNT_SLAB1 ; i++) {
    monorail_pole_setup(&(monorail->_poles_slab1[i]), monorail->_pinStart + (i * 2), 0, 0, LightState::Off);
  }
}


/*
 * 
 */
void monorail_system_loop(MonorailSystem *monorail, unsigned long updateTime) {
  for (int i = 0 ; i < MONORAIL_POLE_COUNT_SLAB1 ; i++) {
    monorail_pole_loop(&(monorail->_poles_slab1[i]), updateTime);
  }
}

/*
 * Called every streetlight control button-down.
 */
void monorail_system_state_increment(MonorailSystem *monorail, unsigned long updateTime) {

  // Cycle through the available states.
  int blinkOffset = 0;
  int blinkOffsetOrangeDelta = 0;
  switch (monorail->_lightState) {
    case LightState::On:
      blinkOffset = BLINK_OFFSET;
      blinkOffsetOrangeDelta = 250;
      monorail->_lightState = LightState::UniformBlink;
      break;
      
    case LightState::UniformBlink:
      monorail->_lightState = LightState::Off;
      break;
      
    case LightState::Off:
    default:
      monorail->_lightState = LightState::On;
      break;
  }

  // Record start time of state machine, and re-init all the lights.
  monorail->_lightStateStartTime = updateTime;
  for (int i = 0 ; i < MONORAIL_POLE_COUNT_SLAB1 ; i++) {
    monorail_pole_setup(&(monorail->_poles_slab1[i]), monorail->_pinStart + (i * 2), 
      (i * blinkOffset), (i * blinkOffset) + blinkOffsetOrangeDelta, monorail->_lightState);
  }

  Serial.print(F("monorail light state now: "));
  Serial.println(monorail->_lightState);
} 
