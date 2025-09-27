#include "Arduino.h"
#include "SeaRobLogger.h"
#include "SliderInput.h"


/**
 * 
 */
void sliderinput_setup(SliderInput *input, String name, int pin, onSliderChange changeHandler) {
  // Init state.
  input->_name = name;
  input->_pin = pin;
  input->_levelPrev = 0;
  input->_onChangeHandler = changeHandler;

  // Tell arduino how we wil lbe using the pin. The slider already has a 5v and G line running to it, and its signal
   // line should be hooked up to our input pin.
  pinMode(input->_pin, INPUT);
  
  bclogger("sliderinput: \"%s\" started on pin %d", input->_name.c_str(), input->_pin);
} 

/*
 */
void sliderinput_loop(SliderInput *input, unsigned long updateTime) {

  // Light Button Control: Detect if the voltage level on the button has changed.
  int currRead = analogRead(input->_pin);   
  int diff = abs(currRead - input->_levelPrev);
  if (diff < 10) {
    //bclogger("sliderinput: \"%s\" no change on pin %d level=%d", input->_name.c_str(), input->_pin, currRead);
    return;
  }
   
  // If this is a button-down event, trigger a state change for the street light subsystem.
  bclogger("sliderinput: \"%s\" triggered change on pin %d, old=%d, new=%d, diff=%d", 
    input->_name.c_str(), input->_pin, input->_levelPrev, currRead, diff);
  input->_onChangeHandler(input, currRead, updateTime);

  // Update the state.
  input->_levelPrev = currRead;
} 
