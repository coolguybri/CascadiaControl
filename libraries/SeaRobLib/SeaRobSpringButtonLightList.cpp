#include "Arduino.h"
#include "SeaRobLogger.h"
#include "SeaRobSpringButtonLightList.h"


	

/*
 */
SeaRobSpringButtonLightList::SeaRobSpringButtonLightList(int numLights, int* buttonPins, int* lightPins, int selectorButtonPin)
 : _numLights(numLights), _blinkState(BlinkState::BlinkState_Off) {
	// Alloc the light array.
	_buttonLights = new SeaRobSpringButtonLight*[_numLights * 2];

	// Init the light+button array.
	int startId = 1;
	for (int i = 0 ; i < _numLights ; i++) {
		SeaRobSpringButtonLight * bl = new SeaRobSpringButtonLight(startId + i, buttonPins[i], lightPins[i], 
				StaticOnButtonDownLightIndividual, this);
		_buttonLights[i] = bl;
	}

    // Blink-mode selector button.
    _buttonModeSelector = new SeaRobSpringButton("pf-light selector", selectorButtonPin, StaticOnButtonDownLightSelector, this);
    
    bclogger("SeaRobSpringButtonLightList: creating with num=%d, state=%d, selector=%d", 
		_numLights, _blinkState, selectorButtonPin);
}


/*
 */
SeaRobSpringButtonLightList::SeaRobSpringButtonLightList(int numLights, int startButtonPin, int startLightPin, int selectorButtonPin)
 : _numLights(numLights), _blinkState(BlinkState::BlinkState_Off) {
	// Alloc the light array.
	_buttonLights = new SeaRobSpringButtonLight*[_numLights];

	// Init the light+button array.
	int startId = 1;
	for (int i = 0 ; i < _numLights ; i++) {
		SeaRobSpringButtonLight * bl = new SeaRobSpringButtonLight(startId + i, startButtonPin + i, startLightPin + i, 
			StaticOnButtonDownLightIndividual, this);
		_buttonLights[i] = bl;
	}

    // Blink-mode selector button.
    _buttonModeSelector = new SeaRobSpringButton("pf-light selector", selectorButtonPin, StaticOnButtonDownLightSelector, this);
    
	bclogger("SeaRobSpringButtonLightList: creating with num=%d, state=%d, button-start=%d, light-start=%d, selector=%d", 
		_numLights, _blinkState, startButtonPin, startLightPin, selectorButtonPin);
}


/*
*/
SeaRobSpringButtonLightList::~SeaRobSpringButtonLightList() {
	bclogger("SeaRobSpringButtonLightList: destroying");
	for (int i = 0 ; i < _numLights ; i++) {
		delete _buttonLights[i];
	}
	delete[] _buttonLights;
	
	delete _buttonModeSelector;
}


/*
 */
void SeaRobSpringButtonLightList::ProcessLoop(unsigned long updateTime) {
  	// Always process button first (which could change our state if it were toggled).
	_buttonModeSelector->ProcessLoop(updateTime);
    
	for (int i = 0 ; i < _numLights ; i++) {
		_buttonLights[i]->ProcessLoop(updateTime);
	}
}


void SeaRobSpringButtonLightList::GetStatusString(char *buf, int buflen) {
	int newlen = snprintf(buf, buflen, "[%d] [", _blinkState);
	for (int i = 0 ; (i < _numLights) && (newlen < buflen) ; i++) {
		buf[newlen++] = _buttonLights[i]->IsOn() ? '*' : 'o';
	}
	if (newlen < buflen)
		buf[newlen++] = ']';
	if (newlen < buflen)
		buf[newlen++] = 0;
}
        
             
/*
 * button callback: any of the individual buttons
 */
void SeaRobSpringButtonLightList::OnButtonDownLightIndividual(long updateTime) {
	bclogger("SeaRobSpringButtonLightList:OnButtonDownLightIndividual, state=%d", _blinkState);
		
	// Turn off any synchronized blinking if we are playing with individual buttons.
	if (_blinkState != BlinkState_Off) {
		_blinkState = BlinkState_Off;
		HandleStateChange(updateTime);
	}
}


/*
 * button callback: selector button
 */
void SeaRobSpringButtonLightList::OnButtonDownLightSelector(long updateTime) {
	bclogger("SeaRobSpringButtonLightList:OnButtonDownLightIndividual, state=%d", _blinkState);
	
	// Incrmement through the states.
	switch (_blinkState) {
	case BlinkState_Off:
		_blinkState = BlinkState_ConstantOn;
		break;
		
	case BlinkState_ConstantOn:
		_blinkState = BlinkState_SyncBlinkShort;
		break;
		
	case BlinkState_SyncBlinkShort:
		_blinkState = BlinkState_SyncBlinkLong;
		break;
		
	case BlinkState_SyncBlinkLong:
		_blinkState = BlinkState_SingleUni;
		break;
	
	case BlinkState_SingleUni:
		_blinkState = BlinkState_SingleUniSlow;
		break;
		
	case BlinkState_SingleUniSlow:
		_blinkState = BlinkState_SingleUniInverse;
		break;
		
	case BlinkState_SingleUniInverse:
		_blinkState = BlinkState_CylonEye;
		break;
		
	case BlinkState_CylonEye:
		_blinkState = BlinkState_Off;
		break;
	}  
	
	HandleStateChange(updateTime);
}


/*
 */
void SeaRobSpringButtonLightList::HandleStateChange(long updateTime) {
	switch (_blinkState) {
	
    case BlinkState_Off: {
		for (int i = 0 ; i < _numLights ; i++) {
			SeaRobSpringButtonLight * bl = _buttonLights[i];
			bl->GetLight()->UpdateState(SeaRobLight::LightState::Off);
		} 
	  }
      break;
      
    case BlinkState_ConstantOn: {
		for (int i = 0 ; i < _numLights ; i++) {
			SeaRobSpringButtonLight * bl = _buttonLights[i];
			bl->GetLight()->UpdateState(SeaRobLight::LightState::On);
		} 
	  }
      break;
  
	case BlinkState_SyncBlinkShort: {
		int delay = 0;
		int durationOn = 500;
		int durationOff = 1000;
		for (int i = 0 ; i < _numLights ; i++) {
			SeaRobSpringButtonLight * bl = _buttonLights[i];
			bl->GetLight()->UpdateBlinkConfig(updateTime, delay, durationOn, durationOff);
			bl->GetLight()->UpdateState(SeaRobLight::LightState::UniformBlink);
		} 
	  }
	  break;
	  
	case BlinkState_SyncBlinkLong: {
		int delay = 0;
		int durationOn = 1500;
		int durationOff = 500;
		for (int i = 0 ; i < _numLights ; i++) {
			SeaRobSpringButtonLight * bl = _buttonLights[i];
			bl->GetLight()->UpdateBlinkConfig(updateTime, delay, durationOn, durationOff);
			bl->GetLight()->UpdateState(SeaRobLight::LightState::UniformBlink);
		} 
	  }
	  break;
	  
   case BlinkState_SingleUni: {
   		// frameTime is the duration in millis of one animation frame.
   		// frameCount is how many frames each light gets in the sequence.
   		// Light is on for 1 frame, then off for the rest of the frameCount (frameCount - 1).
   		// It is also off for every other light's frames.
		int frameTime = 500;
		int frameCount = 2;
		int durationOn = frameTime;
		int durationOff = ((_numLights * frameCount) - 1) * frameTime;
	
		for (int i = 0 ; i < _numLights ; i++) {
			SeaRobSpringButtonLight * bl = _buttonLights[i];
			
			int delay = (i * frameCount) * frameTime;
			bl->GetLight()->UpdateBlinkConfig(updateTime, delay, durationOn, durationOff, false);
			bl->GetLight()->UpdateState(SeaRobLight::LightState::UniformBlink);
		} 
	  }
	  break;
	  
	case BlinkState_SingleUniSlow: {
		int frameTime = 500;
		int frameCount = 3;
		int durationOn = frameTime;
		int durationOff = ((_numLights * frameCount) - 1) * frameTime;
	
		for (int i = 0 ; i < _numLights ; i++) {
			SeaRobSpringButtonLight * bl = _buttonLights[i];
			
			int delay = (i * frameCount) * frameTime;
			bl->GetLight()->UpdateBlinkConfig(updateTime, delay, durationOn, durationOff, false);
			bl->GetLight()->UpdateState(SeaRobLight::LightState::UniformBlink);
		} 
	  }
	  break;

    case BlinkState_SingleUniInverse: {
		int frameTime = 500;
		int frameCount = 4;
		int durationOff = frameTime;
		int durationOn = ((_numLights * frameCount) - 1) * frameTime;
	
		for (int i = 0 ; i < _numLights ; i++) {
			SeaRobSpringButtonLight * bl = _buttonLights[i];
			
			int delay = (i * frameCount) * frameTime;
			bl->GetLight()->UpdateBlinkConfig(updateTime, delay, durationOn, durationOff, true);
			bl->GetLight()->UpdateState(SeaRobLight::LightState::UniformBlink);
		} 
	  }
      break;
      
	case BlinkState_CylonEye: {
		int frameTime = 1000;
	
		for (int i = 0 ; i < _numLights ; i++) {
			SeaRobSpringButtonLight * bl = _buttonLights[i];
			
			int delay = (i * 2) * frameTime;
			int durationOn = frameTime;
			int durationOff = ((_numLights * 2) - 1) * frameTime;
			bl->GetLight()->UpdateBlinkConfig(updateTime, delay, durationOn, durationOff);
			bl->GetLight()->UpdateState(SeaRobLight::LightState::UniformBlink);
		} 
	  }
	  break;

    default:
      bclogger("SeaRobSpringButtonLightList: ERROR: undefined state: %ld", _blinkState);
      break;
  }  

  bclogger("SeaRobSpringButtonLightList: now state=%d", _blinkState);
}
