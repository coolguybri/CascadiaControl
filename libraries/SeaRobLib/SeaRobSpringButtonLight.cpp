#include "Arduino.h"
#include "SeaRobLogger.h"
#include "SeaRobSpringButtonLight.h"


/*
 */
SeaRobSpringButtonLight::SeaRobSpringButtonLight(int uniqueId, int buttonPin, int ledPin, 
		onStateChange stateChangeHandler, void *opaque = NULL) : _light(NULL), _button(NULL) {
	// Record the unique label, so debug messages will make sense later.
	_label = uniqueId;
	_stateChangeHandler = stateChangeHandler;
	_opaque = opaque;
	
	// Setup the light.
	_light = new SeaRobLight(ledPin);
	
	// Setup the button.
	char buttonName[100];
	snprintf(buttonName, 100, "pf-light %02d", _label);
	_button = new SeaRobSpringButton(buttonName, buttonPin, SeaRobSpringButtonLight::StaticOnButtonDown, this);
	
	bclogger("SeaRobSpringButtonLight: button-light %02d: created on button-pin %d and light-pin %d", 
		_label, buttonPin, ledPin);
}


/*
*/
SeaRobSpringButtonLight::~SeaRobSpringButtonLight() {
	bclogger("SeaRobSpringButtonLight: button-light %02d: destroying", _label);
	delete _light;
	delete _button;
}


/*
 */
void SeaRobSpringButtonLight::OnButtonDown(long updateTime) {  
	_light->ToggleOnOff();
	bclogger("SeaRobSpringButtonLight: button-light %02d: toggled to on=%d", _label, _light->IsOn());
	
	// propagate the event to the next layer up.
	_stateChangeHandler(this, updateTime);
}


/*
 */
void SeaRobSpringButtonLight::ProcessLoop(unsigned long updateTime) {
	// Always process button first (which could change our state if it were toggled).
	_button->ProcessLoop(updateTime);
	_light->ProcessLoop(updateTime);
}
