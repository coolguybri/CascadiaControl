#include "Arduino.h"
#include "SeaRobLogger.h"
#include "SeaRobSpringButtonLight.h"


/*
 */
SeaRobSpringButtonLight::SeaRobSpringButtonLight(String name, int buttonPin, int ledPin, 
		bool dimmable, bool useInternalPullUp,
		onStateChange downHandler, onStateChange upHandler, void *opaque) 
		: _name(name), _button(NULL), _dimmable(dimmable),
			_downHandler(downHandler), _upHandler(upHandler), _opaque(opaque), 
			_light(NULL), _extraLightLen(0), _extraLightCapacity(0), _extraLights(NULL)  {
	
	// Setup the optional light.
	if (ledPin >= 0) {	
		_light = new SeaRobLight(ledPin, _dimmable);
	}
	
	// Setup the button.
	char buttonName[255];
	snprintf(buttonName, 255, "%s button", _name.c_str());
	_button = new SeaRobSpringButton(buttonName, buttonPin, useInternalPullUp,
		SeaRobSpringButtonLight::StaticOnButtonDown, SeaRobSpringButtonLight::StaticOnButtonUp, this);
	
	bclogger("SeaRobSpringButtonLight [%d:%s] created on button-pin %d and light-pin %d", 
		_objId, _name.c_str(), buttonPin, ledPin);
}


/*
*/
SeaRobSpringButtonLight::~SeaRobSpringButtonLight() {
	bclogger("SeaRobSpringButtonLight [%d:%s] destroying", 
		_objId, _name.c_str());
	
	for (int i = 0 ; i < _extraLightLen ; i++) {
		delete _extraLights[i];
		_extraLights[i] = NULL;
	}
	delete[] _extraLights;
	
	delete _light;
	delete _button;
}


/*
*/
void SeaRobSpringButtonLight::AddExtraLedPin(int ledPin) {

	if (_extraLightLen == _extraLightCapacity) {
		int newCapacity = (_extraLightCapacity == 0) ? 16 : (_extraLightCapacity * 2);
		SeaRobLight ** newExtraLights = new SeaRobLight *[newCapacity];
		for (int i = 0 ; i < _extraLightLen ; i++) {
			newExtraLights[i] = _extraLights[i];
		}
		for (int i = _extraLightLen ; i < newCapacity ; i++) {
			newExtraLights[i] = NULL;
		}
		
		delete[] _extraLights;
		_extraLightCapacity = newCapacity;
		_extraLights = newExtraLights;
	}
	
	_extraLights[_extraLightLen] = new SeaRobLight(ledPin, _dimmable);
	_extraLightLen++;
}


/*
 */
void SeaRobSpringButtonLight::OnButtonDown(long updateTime) {  
	if (_light) {
		_light->ToggleOnOff();
	}
	for (int i = 0 ; i < _extraLightLen ; i++) {
		_extraLights[i]->ToggleOnOff();
	}
	
	bclogger("SeaRobSpringButtonLight buttondown [%d:%s] toggled to %s", 
		_objId, _name.c_str(), _light->IsOn() ? "on" : "off");
	if (_downHandler) {
		_downHandler(this, updateTime);
	}
}


/*
 */
void SeaRobSpringButtonLight::OnButtonUp(long updateTime) {  
	bclogger("SeaRobSpringButtonLight buttonup [%d:%s] currently set to %s", 
		_objId, _name.c_str(), _light->IsOn() ? "on" : "off");
	if (_upHandler) {
		_upHandler(this, updateTime);
	}
}


/*
 */
void SeaRobSpringButtonLight::ProcessLoop(unsigned long updateTime) {
	// Always process button first (which could change our state if it were toggled).
	_button->ProcessLoop(updateTime);
	
	if (_light) {
		_light->ProcessLoop(updateTime);
	}
	for (int i = 0 ; i < _extraLightLen ; i++) {
		_extraLights[i]->ProcessLoop(updateTime);
	}
}