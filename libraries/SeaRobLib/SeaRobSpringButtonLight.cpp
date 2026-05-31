#include "Arduino.h"
#include "SeaRobLogger.h"
#include "SeaRobSpringButtonLight.h"


/*
 */
SeaRobSpringButtonLight::SeaRobSpringButtonLight(String name, int buttonPin, int ledPin, 
		onStateChange stateChangeHandler, void *opaque, bool dimmable) : _light(NULL), _button(NULL) {
	// Record the unique label, so debug messages will make sense later.
	_name = name;
	_stateChangeHandler = stateChangeHandler;
	_opaque = opaque;
	
	// Setup the light.
	_light = new SeaRobLight(ledPin, dimmable);
	
	// Setup the button.
	char buttonName[255];
	snprintf(buttonName, 255, "%s button", _name.c_str());
	_button = new SeaRobSpringButton(buttonName, buttonPin, SeaRobSpringButtonLight::StaticOnButtonDown, this);
	
	bclogger("SeaRobSpringButtonLight (%d): \"%s\": created on button-pin %d and light-pin %d", 
		_objId, _name.c_str(), buttonPin, ledPin);
}


/*
*/
SeaRobSpringButtonLight::~SeaRobSpringButtonLight() {
	bclogger("SeaRobSpringButtonLight (%d): \"%s\": destroying", _objId, _name.c_str());
	delete _light;
	delete _button;
}


/*
 */
void SeaRobSpringButtonLight::OnButtonDown(long updateTime) {  

	_light->ToggleOnOff();
	bclogger("SeaRobSpringButtonLight (%d): \"%s\": toggled to %s", 
		_objId, _name.c_str(), _light->IsOn() ? "on" : "off");
	
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
