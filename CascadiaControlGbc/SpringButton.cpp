#include "arduino.h"
#include "Logger.h"
#include "SpringButton.h"

/**
 * 
 */
void springbutton_setup(SpringButton *button, String name, int pin, onPressDown downHandler, void *opaque) {
  button->_name = name;
  button->_pin = pin;
  button->_levelPrev = HIGH;
  button->_pressDown = downHandler;
  button->_opaque = opaque;
  
  pinMode(button->_pin, INPUT_PULLUP);
  
  bclogger("springbutton: \"%s\" started on pin %d", button->_name.c_str(), button->_pin);
} 

/*
 */
void springbutton_loop(SpringButton *button, unsigned long updateTime) {

  // Light Button Control: Detect if the voltage level on the button has changed.
  int currRead = digitalRead(button->_pin);   
  if (currRead == button->_levelPrev) {
    //bclogger("springbutton: \"%s\" no change on pin %d level=%d", button->_name.c_str(), button->_pin, currRead);
    return;
  }
   
  // If this is a button-down event, trigger a state change for the street light subsystem.
  if (currRead == LOW) {
    bclogger("springbutton: \"%s\" triggered buttonDown on pin %d", button->_name.c_str(), button->_pin);
    button->_pressDown(button, updateTime);
  } else {
    bclogger("springbutton: \"%s\" triggered buttonUp on pin %d", button->_name.c_str(), button->_pin);
  }

  button->_levelPrev = currRead;
} 
