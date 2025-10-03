#include "Arduino.h"
#include "SeaRobLogger.h"
#include "SeaRobSpringButtonLightList.h"

#define DURATION_ON 500
#define DURATION_OFF 1000
	

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
     /* snprintf(buf, buflen, "%c %c %c %c %c %c %c %c", 
        buttonLights[0]->IsOn() ? '*' : '-', 
        buttonLights[1]->IsOn() ? '*' : '-',
        buttonLights[2]->IsOn() ? '*' : '-', 
        buttonLights[3]->IsOn() ? '*' : '-',
        buttonLights[4]->IsOn() ? '*' : '-', 
        buttonLights[5]->IsOn() ? '*' : '-',
        buttonLights[6]->IsOn() ? '*' : '-', 
        buttonLights[7]->IsOn() ? '*' : '-'); */
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
			_blinkState = BlinkState_SyncBlink;
			break;
		
		case BlinkState_SyncBlink:
			_blinkState = BlinkState_UnSyncBlink;
			break;
		
		case BlinkState_UnSyncBlink:
			_blinkState = BlinkState_UnSyncBlinkSlow;
			break;
		
		case BlinkState_UnSyncBlinkSlow:
			_blinkState = BlinkState_Off;
			break;
	}  
	
	HandleStateChange(updateTime);
}


/*
 */
void SeaRobSpringButtonLightList::HandleStateChange(long updateTime) {
  switch (_blinkState) {
    case BlinkState_SyncBlink:
      for (int i = 0 ; i < _numLights ; i++) {
        SeaRobSpringButtonLight * bl = _buttonLights[i];
        bl->GetLight()->UpdateBlinkConfig(updateTime, 0, DURATION_ON, DURATION_OFF);
        bl->GetLight()->UpdateState(SeaRobLight::LightState::UniformBlink);
      } 
      break;
      
    case BlinkState_UnSyncBlink: {
			int perLightDelay = 200;
			int durationOn = 2000;
			int durationOff = 500;
			for (int i = 0 ; i < _numLights ; i++) {
			  SeaRobSpringButtonLight * bl = _buttonLights[i];
			  bl->GetLight()->UpdateBlinkConfig(updateTime, (i * perLightDelay), durationOn, durationOff);
			  bl->GetLight()->UpdateState(SeaRobLight::LightState::UniformBlink);
			} 
		}
      break;
    
	  case BlinkState_UnSyncBlinkSlow: {
			int perLightDelay = 2000;
			int durationOn = 1000;
			int durationOff = 14000;
			for (int i = 0 ; i < _numLights ; i++) {
			  SeaRobSpringButtonLight * bl = _buttonLights[i];
			  bl->GetLight()->UpdateBlinkConfig(updateTime, (i * perLightDelay), durationOn, durationOff);
			  bl->GetLight()->UpdateState(SeaRobLight::LightState::UniformBlink);
			} 
		}
	  break;
      
    case BlinkState_Off:
      for (int i = 0 ; i < _numLights ; i++) {
        SeaRobSpringButtonLight * bl = _buttonLights[i];
        bl->GetLight()->UpdateState(SeaRobLight::LightState::Off);
      } 
      break;

    case BlinkState_ConstantOn:
      for (int i = 0 ; i < _numLights ; i++) {
        SeaRobSpringButtonLight * bl = _buttonLights[i];
        bl->GetLight()->UpdateState(SeaRobLight::LightState::On);
      } 
      break;

    default:
      bclogger("SeaRobSpringButtonLightList: ERROR: undefined state: %ld", _blinkState);
      break;
  }  

  bclogger("SeaRobSpringButtonLightList: now state=%d", _blinkState);
}
