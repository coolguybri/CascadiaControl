#ifndef __MotorPCM_h__
#define __MotorPCM_h__

#include "arduino.h"

typedef enum {
  MotorState_Off,
  MotorState_Forward,
  MotorState_Reverse,
} MotorPcmState;


/*
 * 
 */
struct MotorPCM {
  String          name;
  MotorPcmState   motorState;
  int             motorPulseWidth; // Between 0-255.
  
  int             pin_input1;
  int             pin_input2;
  int             pin_enable;
};


void motor_setup(MotorPCM *m, String name, int pinInput1, int pinInput2, int pinEnable);
void motor_loop(MotorPCM *m, unsigned long updateTime);

void motor_set_state(MotorPCM *m, MotorPcmState ms);
int motor_set_pulsewidth(MotorPCM *m, int pulseWidth);


#endif // __MotorPCM_h__
