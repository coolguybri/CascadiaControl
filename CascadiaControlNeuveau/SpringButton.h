#ifndef __SpringButton_h__
#define __SpringButton_h__

#include "arduino.h"

struct SpringButton;
typedef int (*onPressDown) (SpringButton *button, long updateTime);

struct SpringButton {
  String          _name;
  int             _pin;
  int            _levelPrev;
  int (*_pressDown)(SpringButton *, long);
};


void springbutton_setup(SpringButton *button, String name, int pin, onPressDown downHandler);
void springbutton_loop(SpringButton *button, unsigned long updateTime);


#endif // __SpringButton_h__
