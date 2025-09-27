#include "Arduino.h"
#include "MonorailSystem.h"

#define BLINK_OFFSET 100


/*
 */
void monorail_pole_setup(MonorailPole *pole, int startPin, int offsetYellow, int offsetOrange, SeaRobLight::LightState lstate) {
  pole->_lightYellow = new SeaRobLight(startPin, offsetYellow);
  pole->_lightYellow->UpdateState(lstate);
  
  pole->_lightOrange = new SeaRobLight(startPin + 1, offsetOrange);
  pole->_lightOrange->UpdateState(lstate);
}



/*
 */
void monorail_pole_destroy(MonorailPole *pole) {
  delete pole->_lightYellow;
  delete pole->_lightOrange;
}

/*
 * 
 */
void monorail_pole_loop(MonorailPole *pole, unsigned long updateTime) {
  pole->_lightYellow->ProcessLoop(updateTime);
  pole->_lightOrange->ProcessLoop(updateTime);
}


/*
 *
 */
void monorail_system_setup(MonorailSystem *monorail, int startPin) {
  monorail->_pinStart = startPin;
  monorail->_lightState = SeaRobLight::LightState::Off;
  monorail->_lightStateStartTime = 0;

  for (int i = 0 ; i < MONORAIL_POLE_COUNT_SLAB1 ; i++) {
    monorail_pole_setup(&(monorail->_poles_slab1[i]), monorail->_pinStart + (i * 2), 0, 0, monorail->_lightState);
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
    case SeaRobLight::LightState::On:
      blinkOffset = BLINK_OFFSET;
      blinkOffsetOrangeDelta = 250;
      monorail->_lightState = SeaRobLight::LightState::UniformBlink;
      break;
      
    case SeaRobLight::LightState::UniformBlink:
      monorail->_lightState = SeaRobLight::LightState::Off;
      break;
      
    case SeaRobLight::LightState::Off:
    default:
      monorail->_lightState = SeaRobLight::LightState::On;
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
