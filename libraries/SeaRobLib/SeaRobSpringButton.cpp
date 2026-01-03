#include "Arduino.h"
#include "SeaRobLogger.h"
#include "SeaRobSpringButton.h"
	  

/*
 */
SeaRobSpringButton::SeaRobSpringButton(String name, int pin, onPressDown downHandler, void *opaque) {
	_name = name;
	_pin = pin;
	_levelPrev = HIGH;
	_pressDownHandler = downHandler;
	_opaque = opaque;
	
	pinMode(_pin, INPUT_PULLUP);
	
	bclogger("SeaRobSpringButton (%d): \"%s\" started on pin %d", _objId, _name.c_str(), _pin);
} 


/*
 */
void SeaRobSpringButton::ProcessLoop(unsigned long updateTime) {

	// Light Button Control: Detect if the voltage level on the button has changed.
	int currRead = digitalRead(_pin);   
	if (currRead == _levelPrev) {
		// bclogger("SeaRobSpringButton (%d): \"%s\" no change on pin %d level=%d", 
		// _objId, button->_name.c_str(), button->_pin, currRead);
		return;
	}
	
	// If this is a button-down event, trigger a state change for the street light subsystem.
	if (currRead == LOW) {
		bclogger("SeaRobSpringButton (%d): \"%s\" triggered buttonDown on pin %d", 
			_objId, this->_name.c_str(), this->_pin);
		_pressDownHandler(this, updateTime);
	} else {
		//bclogger("SeaRobSpringButton (%d): \"%s\" triggered buttonUp on pin %d", 
		//	_objId, this->_name.c_str(), this->_pin);
	}

  	_levelPrev = currRead;
} 
