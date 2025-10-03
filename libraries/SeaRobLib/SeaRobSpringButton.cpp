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
	
	bclogger("SeaRobSpringButton: \"%s\" started on pin %d", _name.c_str(), _pin);
} 


/*
 */
void SeaRobSpringButton::ProcessLoop(unsigned long updateTime) {

	// Light Button Control: Detect if the voltage level on the button has changed.
	int currRead = digitalRead(_pin);   
	if (currRead == _levelPrev) {
		//bclogger("SeaRobSpringButton: \"%s\" no change on pin %d level=%d", button->_name.c_str(), button->_pin, currRead);
		return;
	}
	
	// If this is a button-down event, trigger a state change for the street light subsystem.
	if (currRead == LOW) {
		bclogger("SeaRobSpringButton: \"%s\" triggered buttonDown on pin %d", this->_name.c_str(), this->_pin);
		_pressDownHandler(this, updateTime);
	} else {
		bclogger("SeaRobSpringButton: \"%s\" triggered buttonUp on pin %d", this->_name.c_str(), this->_pin);
	}

  	_levelPrev = currRead;
} 
