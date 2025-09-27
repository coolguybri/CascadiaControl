#include "Arduino.h"
#include "SeaRobLogger.h"
#include "SeaRobSpringButton.h"


/*
 */
SeaRobSpringButton::SeaRobSpringButton(String name, int pin, onPressDown downHandler, void *opaque) {
  this->_name = name;
  this->_pin = pin;
  this->_levelPrev = HIGH;
  this->_pressDownHandler = downHandler;
  this->_opaque = opaque;
  
  pinMode(this->_pin, INPUT_PULLUP);
  
  bclogger("SeaRobSpringButton: \"%s\" started on pin %d", this->_name.c_str(), this->_pin);
} 


/*
 */
void SeaRobSpringButton::ProcessLoop(unsigned long updateTime) {

  // Light Button Control: Detect if the voltage level on the button has changed.
  int currRead = digitalRead(this->_pin);   
  if (currRead == this->_levelPrev) {
    //bclogger("SeaRobSpringButton: \"%s\" no change on pin %d level=%d", button->_name.c_str(), button->_pin, currRead);
    return;
  }
   
  // If this is a button-down event, trigger a state change for the street light subsystem.
  if (currRead == LOW) {
    bclogger("SeaRobSpringButton: \"%s\" triggered buttonDown on pin %d", this->_name.c_str(), this->_pin);
    this->_pressDownHandler(this, updateTime);
  } else {
    bclogger("SeaRobSpringButton: \"%s\" triggered buttonUp on pin %d", this->_name.c_str(), this->_pin);
  }

  this->_levelPrev = currRead;
} 
