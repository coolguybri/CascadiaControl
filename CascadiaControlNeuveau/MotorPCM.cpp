#include "arduino.h"
#include "Logger.h"
#include "MotorPCM.h"

/*
 * 
 */
void motor_setup(struct MotorPCM *m, String name, int pinInput1, int pinInput2, int pinEnable) {
  m->name = name;
  m->motorState = MotorState_Off;
  m->motorPulseWidth = 0;
  m->pin_input1 = pinInput1;
  m->pin_input2 = pinInput2;
  m->pin_enable = pinEnable;
  
  pinMode(m->pin_input1, OUTPUT);
  pinMode(m->pin_input2, OUTPUT);
  pinMode(m->pin_enable, OUTPUT);

  bclogger("motor_setup: \"%s\" pins: in1=%d, in2=%d, enb=%d, state=%d",
    m->name.c_str(), m->pin_input1, m->pin_input2, m->pin_enable, m->motorState);
}

/*
 * 
 */
void motor_loop(MotorPCM *m, unsigned long updateTime) {  
  int in1 = LOW;
  int in2 = LOW;
  switch (m->motorState) {
  case MotorState_Forward:
      in1 = HIGH;
      break;
  case MotorState_Reverse:
      in2 = HIGH;
      break;
  }
  
  // write out current state.
  digitalWrite(m->pin_input1, in1);
  digitalWrite(m->pin_input2, in2);
  analogWrite(m->pin_enable, m->motorPulseWidth);
}

/*
 * 
 */
void motor_set_state(MotorPCM *m, MotorPcmState ms) {
  m->motorState = ms;
  bclogger("motor_set_state: \"%s\" state=%d", m->name.c_str(), m->motorState);
}

/*
 * speed must be 0-255.
 * https://www.arduino.cc/reference/tr/language/functions/analog-io/analogwrite/
 */
void motor_set_pulsewidth(MotorPCM *m, int pulseWidth) {
  if (pulseWidth < 0) 
    pulseWidth = 0;
  if (pulseWidth > 255) 
    pulseWidth = 255;
  
  m->motorPulseWidth = pulseWidth;
  bclogger("motor_set_pulsewidth: \"%s\" width=%d", m->name.c_str(), m->motorPulseWidth);
}
