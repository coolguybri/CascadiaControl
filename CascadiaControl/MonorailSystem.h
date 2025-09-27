#ifndef __Monorail_h__
#define __Monorail_h__

#include "SeaRobLight.h"

#define MONORAIL_POLE_COUNT_SLAB1         7

struct MonorailPole {
  SeaRobLight *       _lightYellow;
  SeaRobLight *       _lightOrange;

  unsigned long       blinkTime;
  bool                blinkState;
  int                 blinkIntervalOn;
  int                 blinkIntervalOff;
};


struct MonorailSystem {
    int                       _pinStart;
    
    SeaRobLight::LightState   _lightState;
    unsigned long             _lightStateStartTime;
    
    MonorailPole              _poles_slab1[MONORAIL_POLE_COUNT_SLAB1];
};


void monorail_system_setup(MonorailSystem *monorail, int startPin);

void monorail_system_loop(MonorailSystem *monorail, unsigned long updateTime);

void monorail_system_state_increment(MonorailSystem *monorail, unsigned long updateTime);

#endif // __Monorail_h__
