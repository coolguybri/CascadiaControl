#include "Arduino.h"
#include "SeaRobLight.h"
#include "SeaRobLogger.h"


#define BLINK_INTERVAL_ON 2000
#define BLINK_INTERVAL_OFF 250


/*
 */
SeaRobLight::SeaRobLight(int p, int offset): _pin(p) {
  _state = LightState::Off;
  _blinkOffset = offset;
  _blinkIntervalOn = BLINK_INTERVAL_ON;
  _blinkIntervalOff = BLINK_INTERVAL_OFF;
  _blinkTimeNext = 0;
  _litState = false;
  _loggingState = false;

  unsigned long now = millis();
  if (_blinkOffset > 0) {
    _blinkTimeNext = (now + _blinkOffset);
  }

  pinMode(_pin, OUTPUT);
 
  bclogger("SeaRobLight: pin=%d, state=%d, offset=%d, nextblink=%lu", 
    _pin, _state, _blinkOffset, _blinkTimeNext);
}


/*
 */
void SeaRobLight::SetDebugLogging(bool setter) {
  bool prev = _loggingState;
  _loggingState = setter;

  if (_loggingState != prev) {
    bclogger("SeaRobLight: pin=%d, logging=%b", _pin, _loggingState);
  }
}


/*
 */
void SeaRobLight::UpdateState(LightState state) {
   _state = state;

   if (_loggingState) {
      bclogger("SeaRobLight::UpdateState: pin=%d, state=%d", _pin, _state);
   }
}


/*
 */
void SeaRobLight::UpdateBlinkConfig(unsigned long startTime, int offset, int durationOn, int durationOff) {
   _blinkOffset = offset;
   _blinkIntervalOn = durationOn;
   _blinkIntervalOff = durationOff;
   _blinkTimeNext = startTime + _blinkOffset;

   if (_loggingState) {
     bclogger("SeaRobLight::UpdateBlinkConfig: pin=%d, state=%d, offset=%d, startTime=%lu, nextblink=%lu, duration=%d/%d", 
        _pin, _state, _blinkOffset, startTime, _blinkTimeNext, _blinkIntervalOn, _blinkIntervalOff);
   }
}


/*
 */
void SeaRobLight::ToggleOnOff() {
   switch (_state) {
    case LightState::Off:
      _state = LightState::On;
      break;
      
    case LightState::On: 
      _state = LightState::Off;
      break;
    
    default:
      if (_loggingState) {
        bclogger("SeaRobLight::ToggleOnOff: pin=%d, jumping to off from %d", _pin, _state);
      }
      _state = LightState::Off;
      break;
   }

   if (_loggingState) {
      bclogger("SeaRobLight::ToggleOnOff: pin=%d, state=%d", _pin, _state);
   }
}


/* 
 */
boolean SeaRobLight::IsOn() {
  return _litState;
}


/*
 */
String SeaRobLight::GetStateName() {
  switch (_state) {
    case LightState::Off:
      return "off";
      
    case LightState::On: 
     return "on";

    case LightState::UniformBlink:
      return "blink";
    
    default:
      bclogger("SeaRobLight::GetStateName: pin=%d, ILLEGAL STATE CHANGE", _pin);
      return "illegal-state";
   }
}


/*
 */
void SeaRobLight::ProcessLoop(unsigned long updateTime) {
  switch (_state) {
    case LightState::Off:
      _litState = false;
      break;
      
    case LightState::On: 
      _litState = true;
      break;
    
    case LightState::UniformBlink:
      if (updateTime >= _blinkTimeNext) {
          unsigned long thisTime = _blinkTimeNext;
          
          int nextDuration = _litState ? _blinkIntervalOn : _blinkIntervalOff;
          _litState = !_litState;
          _blinkTimeNext = _blinkTimeNext + nextDuration;
          
          if (_loggingState) {
              bclogger("light_loop: pin=%d, state=%d, offset=%d, nextDur=%d, thisblink=%lu, nextblink=%lu, update=%lu", 
                _pin, _state, _blinkOffset, nextDuration, thisTime, _blinkTimeNext, updateTime);
          }
      }
      break;
  }

  // Write out current state to the led.
  digitalWrite(_pin, _litState);
}
