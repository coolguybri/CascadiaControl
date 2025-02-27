#ifndef __SliderInput_h__
#define __SliderInput_h__

#include "arduino.h"

struct SliderInput;
typedef int (*onSliderChange) (SliderInput *input, int newValue, long updateTime);

struct SliderInput {
  String          _name;
  int             _pin;
  int             _levelPrev;
  onSliderChange  _onChangeHandler;
};


void sliderinput_setup(SliderInput *input, String name, int pin, onSliderChange changeHandler);
void sliderinput_loop(SliderInput *input, unsigned long updateTime);

#endif // __SliderInput_h__
