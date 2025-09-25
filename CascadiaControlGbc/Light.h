
#ifndef __Light_h__
#define __Light_h__

// Data Sructures: enum for the state of the lights.
typedef enum {
  Off = 0,
  On,
  UniformBlink,
} LightState;

/*
 * 
 */
struct Light {
  int                 pin;
  LightState          state;
  int                 blinkOffset;
  int                 blinkIntervalOn;
  int                 blinkIntervalOff;

  unsigned long       blinkTimeNext;
  bool                blinkState;
};


void light_setup(Light *l, int pin, LightState state, int offset);

void light_update_state(Light *l, LightState state);
void light_update_blink(Light *l, unsigned long startTime, int offset, int durationOn, int durationOff);

void light_toggle_onoff(Light *l);
boolean light_ison(Light *l);
String light_get_state_name(Light *l);

void light_loop(Light *l, unsigned long updateTime);

#endif // __Light_h__
