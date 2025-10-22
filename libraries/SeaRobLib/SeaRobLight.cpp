#include "Arduino.h"
#include "SeaRobLight.h"
#include "SeaRobLogger.h"


/*
 */
SeaRobLight::SeaRobLight(int p, int offset): _pin(p) {
  _state = LightState::Off;
  _blinkOffset = offset;
  _blinkDurationCount = 0;
  _blinkDurationIndex = 0;
  _blinkDurations = NULL;
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
SeaRobLight::~SeaRobLight() {
	delete[] _blinkDurations;
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
void SeaRobLight::UpdateBlinkConfig(unsigned long startTime, int offset, int durationOn, int durationOff, boolean startOn) {
	int durations[2] = { durationOn, durationOff };
	UpdateBlinkSequenceConfig(startTime, offset, 2, durations, startOn);
}


void SeaRobLight::UpdateBlinkSequenceConfig(unsigned long startTime, int offset, int durationCount, int *durations, boolean startOn) {
	// Set initial state.
	_blinkOffset = offset;
	_blinkTimeNext = startTime + _blinkOffset;
	_litState = startOn;
	
	// Setup the blink state, deleting any previous state.
	if (_blinkDurations) {
		delete[] _blinkDurations;
		_blinkDurations = NULL;
	}
	
	_blinkDurationCount = durationCount;
	_blinkDurationIndex = 0;
	_blinkDurations = new int[durationCount];
	for (int i = 0 ; i < _blinkDurationCount ; i++) {
		_blinkDurations[i] = durations[i];
		if (_loggingState) {
			bclogger("SeaRobLight::UpdateBlinkConfig: pin=%d, state=%d, durations_index=%d, duration=%d", 
				_pin, _state, i, _blinkDurations[i]);
		}
	}
	
	if (_loggingState) {
		bclogger("SeaRobLight::UpdateBlinkConfig: pin=%d, state=%d, durations=%d, offset=%d, startTime=%lu, nextblink=%lu", 
			_pin, _state, _blinkDurationCount, _blinkOffset, startTime, _blinkTimeNext);
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
		
			_litState = !_litState;
		
			int nextDuration = 1000;
			if (!_blinkDurations) {
				bclogger("light_loop: pin=%d, state=%d, ILLEGAL STATE - no durations", _pin, _state);
			} else {
				nextDuration = _blinkDurations[_blinkDurationIndex];
				if (_loggingState) {
					bclogger("light_loop: duration index %d = %d", _blinkDurationIndex, nextDuration);
				}

				_blinkDurationIndex++;
				if (_blinkDurationIndex >= _blinkDurationCount)
					_blinkDurationIndex = 0;
					
				if (_loggingState) {
					bclogger("light_loop: next index is %d", _blinkDurationIndex);
				}
			}
		
		_blinkTimeNext = _blinkTimeNext + nextDuration;
		
		if (_loggingState) {
			bclogger("light_loop: update=%lu, pin=%d, state=%d, nextDur=%d, thisFrameStart=%lu, nextFrameStart=%lu", 
				updateTime, _pin, _state, nextDuration, thisTime, _blinkTimeNext);
		}
	  }
      break;
  }

  // Write out current state to the led.
  digitalWrite(_pin, _litState);
}
