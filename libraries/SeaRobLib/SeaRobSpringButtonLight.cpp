#include "Arduino.h"
#include "SeaRobLogger.h"
#include "SeaRobSpringButtonLight.h"


/*
 */
SeaRobSpringButtonLight::SeaRobSpringButtonLight(int uniqueId, int buttonPin, int ledPin, onStateChange stateChangeHandler):
		 _light(NULL), _button(NULL) {
  // Record the unique label, so debug messages will make sense later.
  this->_label = uniqueId;
  this->_stateChangeHandler = stateChangeHandler;

  // Setup the light.
  this->_light = new SeaRobLight(ledPin);

  // Setup the button.
  char buttonName[100];
  snprintf(buttonName, 100, "pf-light %02d", this->_label);
  this->_button = new SeaRobSpringButton(buttonName, buttonPin, SeaRobSpringButtonLight::StaticOnButtonDown, this);
  
  bclogger("SeaRobSpringButtonLight: button-light %02d: created on button-pin %d and light-pin %d", 
  	this->_label, buttonPin, ledPin);
}


/*
*/
SeaRobSpringButtonLight::~SeaRobSpringButtonLight() {
	bclogger("SeaRobSpringButtonLight: button-light %02d: destroying", this->_label);
	delete this->_light;
	delete this->_button;
}


/*
 */
void SeaRobSpringButtonLight::OnButtonDown(long updateTime) {  
  this->_light->ToggleOnOff();
  bclogger("SeaRobSpringButtonLight: button-light %02d: toggled to %d", this->_label, this->_light->IsOn());

  // propagate the event to the next layer up.
  _stateChangeHandler(this, updateTime);
}


/*
 */
void SeaRobSpringButtonLight::ProcessLoop(unsigned long updateTime) {
  // Always process button first (which could change our state if it were toggled).
  this->_button->ProcessLoop(updateTime);
  this->_light->ProcessLoop(updateTime);
}
