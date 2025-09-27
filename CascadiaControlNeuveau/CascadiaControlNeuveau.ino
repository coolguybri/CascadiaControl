#include "Arduino.h"
#include "SeaRobDisplay.h"
#include "SeaRobLight.h"
#include "SeaRobLogger.h"
#include "SeaRobSpringButton.h"

#include "MotorPCM.h"
#include "SliderInput.h"


// Constants: Specific I/O Pins that must be used.
// Assumes the Arduino Mega 3560 R3 Board.

// Windmill Subsystem
#define PIN_WINDMILL_BUTTON_PWR     22 // Digital Pin, input
#define PIN_WINDMILL_BUTTON_DIR     23 // Digital Pin, input
#define PIN_WINDMILL_BUTTON_INC     24 // Digital Pin, input
#define PIN_WINDMILL_BUTTON_DEC     25 // Digital Pin, input
#define PIN_WINDMILL_MOTOR_IN1      52 // Digital Pin, output
#define PIN_WINDMILL_MOTOR_IN2      53 // Digital Pin, output
#define PIN_WINDMILL_MOTOR_ENB      2  // Digital PWM Pin, output

// Train subsystem
#define PIN_TRAIN_BUTTON_PWR        26 // Digital Pin, input
#define PIN_TRAIN_BUTTON_DIR        27 // Digital Pin, input
#define PIN_TRAIN_BUTTON_INC        28 // Digital Pin, input
#define PIN_TRAIN_BUTTON_DEC        29 // Digital Pin, input
#define PIN_TRAIN_MOTOR_IN1         50 // Digital Pin, output
#define PIN_TRAIN_MOTOR_IN2         51 // Digital Pin, output
#define PIN_TRAIN_MOTOR_ENB         3  // Digital PWM Pin, output

#define PIN_TRAIN_SLIDER            A1 // Analog Pin, input (experimental)

// Lego PowerFunctions Light Array
#define PIN_PF_LIGHT_BUTTON_1       30 // Digital Pin, input
#define PIN_PF_LIGHT_BUTTON_2       31 // Digital Pin, input
#define PIN_PF_LIGHT_BUTTON_3       32 // Digital Pin, input
#define PIN_PF_LIGHT_BUTTON_4       33 // Digital Pin, input
#define PIN_PF_LIGHT_BUTTON_5       34 // Digital Pin, input
#define PIN_PF_LIGHT_BUTTON_6       35 // Digital Pin, input
#define PIN_PF_LIGHT_CTRL_1         40 // Digital Pin, output
#define PIN_PF_LIGHT_CTRL_2         41 // Digital Pin, output
#define PIN_PF_LIGHT_CTRL_3         44 // Digital Pin, output (switched with 42)
#define PIN_PF_LIGHT_CTRL_4         43 // Digital Pin, output
#define PIN_PF_LIGHT_CTRL_5         42 // Digital Pin, output (switched with 44)

// LCD Display subsystem
#define PIN_I2C_SDA                 20 // Dedicated SDA Output Pin (mega only)
#define PIN_I2C_SCL                 21 // Dedicated SCL Output Pin (mega only)


// Global Variables: Global Run state of the entire aplication.
unsigned long   startTime = 0; 
unsigned long   lastUpdateTime = 0;
String          buildName = String("BasementControl");
const char      buildTimestamp[] =  __DATE__ " " __TIME__;
const char      buildDatestamp[] = __DATE__;


// Globals: OLED Display subsystem, connected via I2C interface
boolean useDisplay = true; 
SeaRobDisplay display(PIN_I2C_SDA, PIN_I2C_SCL);


// Globals: Windmill subsystem.
boolean       useWindmill = true;
boolean       windmillPower = false;
boolean       windmillDirection = true;
int           windmillVelocity = 120;
MotorPCM      motorWindmill;
SeaRobSpringButton *  buttonWindmillPwr;
SeaRobSpringButton *  buttonWindmillDir;
SeaRobSpringButton *  buttonWindmillInc;
SeaRobSpringButton *  buttonWindmillDec;


// Globals: Train subsystem.
boolean       useTrain = true;
boolean       trainPower = false;
boolean       trainDirection = true;
int           trainVelocity = 200;
MotorPCM      motorTrain;
SeaRobSpringButton *  buttonTrainPwr;
SeaRobSpringButton *  buttonTrainDir;
SeaRobSpringButton *  buttonTrainInc;
SeaRobSpringButton *  buttonTrainDec;
SliderInput   sliderTrain;


// Globals: PowerFunctions (PF) Lights
boolean               usePFLight = true;
SeaRobLight *         lightStation1;
SeaRobLight *         lightStation2;
SeaRobLight *         lightBugle1;
SeaRobLight *         lightStorm1;
SeaRobLight *         lightStorm2;
SeaRobSpringButton *  buttonPFLight1;
SeaRobSpringButton *  buttonPFLight2;
SeaRobSpringButton *  buttonPFLight3;
SeaRobSpringButton *  buttonPFLight4;
SeaRobSpringButton *  buttonPFLight5;
SeaRobSpringButton *  buttonPFLight6;


/*
 * Windmill button callbacks
 */
 
void onButtonDownWindmillPwr(SeaRobSpringButton *button, long updateTime) {
  windmillPower = !windmillPower;
  bclogger("windmill power: toggled to %d", windmillPower);

  if (windmillPower) {
    motor_set_state(&motorWindmill, windmillDirection ? MotorState_Forward : MotorState_Reverse);
  } else {
    motor_set_state(&motorWindmill, MotorState_Off);
  }
}

void onButtonDownWindmillDir(SeaRobSpringButton *button, long updateTime) {
  windmillDirection = !windmillDirection;
  bclogger("windmill direction: toggled to %d", windmillDirection);

  if (windmillPower) {
    motor_set_state(&motorWindmill, windmillDirection ? MotorState_Forward : MotorState_Reverse);
  } else {
    motor_set_state(&motorWindmill, MotorState_Off);
  }
}

void onButtonDownWindmillInc(SeaRobSpringButton *button, long updateTime) {
  windmillVelocity += 20;
  windmillVelocity = motor_set_pulsewidth(&motorWindmill, windmillVelocity);
  bclogger("windmill inc: %d", windmillVelocity);
}

void onButtonDownWindmillDec(SeaRobSpringButton *button, long updateTime) {
  windmillVelocity -= 20;
  windmillVelocity = motor_set_pulsewidth(&motorWindmill, windmillVelocity);
  bclogger("windmill dec: %d", windmillVelocity);
}


/*
 * Train button callbacks
 */
 
void onButtonDownTrainPwr(SeaRobSpringButton *button, long updateTime) {
  trainPower = !trainPower;
  bclogger("train power: toggled to %d", trainPower);

  if (trainPower) {
    motor_set_state(&motorTrain, trainDirection ? MotorState_Forward : MotorState_Reverse);
  } else {
    motor_set_state(&motorTrain, MotorState_Off);
  }
}

void onButtonDownTrainDir(SeaRobSpringButton *button, long updateTime) {
  trainDirection = !trainDirection;
  bclogger("train direction: toggled to %d", trainDirection);

  if (trainPower) {
    motor_set_state(&motorTrain, trainDirection ? MotorState_Forward : MotorState_Reverse);
  } else {
    motor_set_state(&motorTrain, MotorState_Off);
  }
}

int onButtonDownTrainInc(SeaRobSpringButton *button, long updateTime) {
  trainVelocity += 20;
  trainVelocity = motor_set_pulsewidth(&motorTrain, trainVelocity);
  bclogger("train inc: %d", trainVelocity);
}

int onButtonDownTrainDec(SeaRobSpringButton *button, long updateTime) {
  trainVelocity -= 20;
  trainVelocity = motor_set_pulsewidth(&motorTrain, trainVelocity);
  bclogger("train dec: %d", trainVelocity);
}


int onSliderChangeTrain(SliderInput *input, int newValue, long updateTime) {
  //trainDirection = !trainDirection;
  bclogger("train velocity: set to %d", newValue);

  // TODO: This doesnt work yet.
  // Translate from 104 ratio to 2256 raio. Store as float?
}


/*
 * pf-light button callbacks
 */

 typedef enum {
  BlinkState_Off = 0,
  BlinkState_SyncBlink,
  BlinkState_UnSyncBlink,
} BlinkState;

BlinkState bstate;
 
void onButtonDownLightStation1(SeaRobSpringButton *button, long updateTime) {
  lightStation1->ToggleOnOff();
  bstate = BlinkState_Off;
  bclogger("lightStation1: toggled to %d", lightStation1->GetStateName());
}

void onButtonDownLightStation2(SeaRobSpringButton *button, long updateTime) {
  lightStation2->ToggleOnOff();
  bstate = BlinkState_Off;
  bclogger("lightStation2: toggled to %d", lightStation2->GetStateName());
}

void onButtonDownLightBugle1(SeaRobSpringButton *button, long updateTime) {
  lightBugle1->ToggleOnOff();
  bstate = BlinkState_Off;
  bclogger("lightBugle1: toggled to %d", lightBugle1->GetStateName());
}

void onButtonDownLightStorm1(SeaRobSpringButton *button, long updateTime) {
  lightStorm1->ToggleOnOff();
  bstate = BlinkState_Off;
  bclogger("lightStorm1: toggled to %d", lightStorm1->GetStateName());
}

void onButtonDownLightStorm2(SeaRobSpringButton *button, long updateTime) {
  lightStorm2->ToggleOnOff();
  bstate = BlinkState_Off;
  bclogger("lightStorm2: toggled to %d", lightStorm2->GetStateName());
}

#define DURATION_ON 500
#define DURATION_OFF 1000

void onButtonDownLightSelector(SeaRobSpringButton *button, long updateTime) {
  
  switch (bstate) {
    case BlinkState_Off:
      bstate = BlinkState_SyncBlink;
      lightStation1->UpdateBlinkConfig(updateTime, 0, DURATION_ON, DURATION_OFF);
      lightStation1->UpdateState(SeaRobLight::LightState::UniformBlink);
      
      lightStation2->UpdateBlinkConfig(updateTime, 0, DURATION_ON, DURATION_OFF);
      lightStation2->UpdateState(SeaRobLight::LightState::UniformBlink);
      
      lightStorm1->UpdateBlinkConfig(updateTime, 0, DURATION_ON, DURATION_OFF);
      lightStorm1->UpdateState(SeaRobLight::LightState::UniformBlink);
      
      lightStorm2->UpdateBlinkConfig(updateTime, 0, DURATION_ON, DURATION_OFF);
      lightStorm2->UpdateState(SeaRobLight::LightState::UniformBlink);
      break;
      
    case BlinkState_SyncBlink:
      bstate = BlinkState_UnSyncBlink;
      lightStation1->UpdateBlinkConfig(updateTime, 0, DURATION_ON, DURATION_OFF);
      lightStation1->UpdateState(SeaRobLight::LightState::UniformBlink);
      lightStation2->UpdateBlinkConfig(updateTime, 125, DURATION_ON, DURATION_OFF);
      lightStation2->UpdateState(SeaRobLight::LightState::UniformBlink);
      lightStorm1->UpdateBlinkConfig(updateTime, 250, DURATION_ON, DURATION_OFF);
      lightStorm1->UpdateState(SeaRobLight::LightState::UniformBlink);
      lightStorm2->UpdateBlinkConfig(updateTime, 375, DURATION_ON, DURATION_OFF);
      lightStorm2->UpdateState(SeaRobLight::LightState::UniformBlink);
      break;
      
    case BlinkState_UnSyncBlink:
      bstate = BlinkState_Off;
      lightStation1->UpdateState(SeaRobLight::LightState::Off);
      lightStation2->UpdateState(SeaRobLight::LightState::Off);
      lightStorm1->UpdateState(SeaRobLight::LightState::Off);
      lightStorm2->UpdateState(SeaRobLight::LightState::Off);
      break;
  }  

  bclogger("lightSelector: now set to %d", bstate);
}


/*
 * Entrypoint: called once when the program first starts, just to initialize all the sub-components.
 */
void setup() {  
  
  // Record what time we started.
  startTime = millis();
  
  // Init the serial line; important for debug messages back to the Arduino Serial Monitor.
  Serial.begin(9600);

  bclogger(""); // Skip line to seperate from last instance of the program.
  bclogger("setup: begin \"%s\" (build: %s)", buildName.c_str(), buildTimestamp);

  if (useDisplay) {
    bclogger("setup: OLED start...");
    display.setup(buildTimestamp);
    bclogger("setup: OLED complete");
  }

  if (useWindmill) {
    bclogger("setup: windmill start...");
    
    motor_setup(&motorWindmill, "windmill", PIN_WINDMILL_MOTOR_IN1, PIN_WINDMILL_MOTOR_IN2, PIN_WINDMILL_MOTOR_ENB);
    windmillVelocity = motor_set_pulsewidth(&motorWindmill, windmillVelocity); // middle value.
    
    buttonWindmillPwr = new SeaRobSpringButton("windmill power", PIN_WINDMILL_BUTTON_PWR, &onButtonDownWindmillPwr);
    buttonWindmillDir = new SeaRobSpringButton("windmill direction", PIN_WINDMILL_BUTTON_DIR, &onButtonDownWindmillDir);
    buttonWindmillInc = new SeaRobSpringButton("windmill speed inc", PIN_WINDMILL_BUTTON_INC, &onButtonDownWindmillInc);
    buttonWindmillDec = new SeaRobSpringButton("windmill speed dec", PIN_WINDMILL_BUTTON_DEC, &onButtonDownWindmillDec);

    bclogger("setup: windmill complete, power=%d, dir=%d, v=%d", windmillPower, windmillDirection, windmillVelocity);
  }

  if (useTrain) {
    bclogger("setup: train start...");
    
    motor_setup(&motorTrain, "train", PIN_TRAIN_MOTOR_IN1, PIN_TRAIN_MOTOR_IN2, PIN_TRAIN_MOTOR_ENB);
    trainVelocity = motor_set_pulsewidth(&motorTrain, trainVelocity); // middle value.
    
    buttonTrainPwr = new SeaRobSpringButton("train power", PIN_TRAIN_BUTTON_PWR, &onButtonDownTrainPwr);
    buttonTrainDir = new SeaRobSpringButton("train direction", PIN_TRAIN_BUTTON_DIR, &onButtonDownTrainDir);
    buttonTrainInc = new SeaRobSpringButton("train up", PIN_TRAIN_BUTTON_INC, &onButtonDownTrainInc);
    buttonTrainDec = new SeaRobSpringButton("train down", PIN_TRAIN_BUTTON_DEC, &onButtonDownTrainDec);
    //sliderinput_setup(&sliderTrain, "train velocity", PIN_TRAIN_SLIDE, &onSliderChangeTrain);

    bclogger("setup: train complete, power=%d, dir=%d, v=%d", trainPower, trainDirection, trainVelocity);
  }

  if (usePFLight) {
    bclogger("setup: pf-light start...");
    
    lightStation1 = new SeaRobLight(PIN_PF_LIGHT_CTRL_1);
    lightStation2 = new SeaRobLight(PIN_PF_LIGHT_CTRL_2);
    lightBugle1 = new SeaRobLight(PIN_PF_LIGHT_CTRL_3);
    lightStorm1 = new SeaRobLight(PIN_PF_LIGHT_CTRL_4);
    lightStorm2 = new SeaRobLight(PIN_PF_LIGHT_CTRL_5);
    
    buttonPFLight1 = new SeaRobSpringButton("pf-light station1", PIN_PF_LIGHT_BUTTON_1, &onButtonDownLightStation1);
    buttonPFLight2 = new SeaRobSpringButton("pf-light station2", PIN_PF_LIGHT_BUTTON_2, &onButtonDownLightStation2);
    buttonPFLight3 = new SeaRobSpringButton("pf-light bugle1", PIN_PF_LIGHT_BUTTON_3, &onButtonDownLightBugle1);
    buttonPFLight4 = new SeaRobSpringButton("pf-light storm1", PIN_PF_LIGHT_BUTTON_4, &onButtonDownLightStorm1);
    buttonPFLight5 = new SeaRobSpringButton("pf-light storm2", PIN_PF_LIGHT_BUTTON_5, &onButtonDownLightStorm2);
    
    buttonPFLight6 = new SeaRobSpringButton("pf-light selector", PIN_PF_LIGHT_BUTTON_6, &onButtonDownLightSelector);

    bclogger("setup: pf-light complete, mode=%d/%d/%d/%d/%d", 
      lightStation1->GetStateName(), lightStation2->GetStateName(), 
      lightBugle1->GetStateName(), 
      lightStorm1->GetStateName(), lightStorm2->GetStateName());
  }

  // Init the rest of our internal state.
  bclogger("setup: complete for \"%s\"", buildName.c_str());
}


/**
 * Main Loop: called over and over again as the robot runs.
 * Disopatches this event to all the subsystems.
 */
void loop() {

  // Get the current time.
  lastUpdateTime = millis();

  if (useWindmill) {
      // Process inputs first so they have immediate impact.
      buttonWindmillPwr->ProcessLoop(lastUpdateTime);
      buttonWindmillDir->ProcessLoop(lastUpdateTime);
      buttonWindmillInc->ProcessLoop(lastUpdateTime);
      buttonWindmillDec->ProcessLoop(lastUpdateTime);
    
      // Increment the rest of the state machines.
      motor_loop(&motorWindmill, lastUpdateTime);
  }

  if (useTrain) {
      // Process inputs first so they have immediate impact.
      buttonTrainPwr->ProcessLoop(lastUpdateTime);
      buttonTrainDir->ProcessLoop(lastUpdateTime);
      buttonTrainInc->ProcessLoop(lastUpdateTime);
      buttonTrainDec->ProcessLoop(lastUpdateTime);
      //sliderinput_loop(&sliderTrain, lastUpdateTime);

      // Increment the rest of the state machines.
      motor_loop(&motorTrain, lastUpdateTime);
  }

  if (usePFLight) {
      // Process inputs first so they have immediate impact.
      buttonPFLight1->ProcessLoop(lastUpdateTime);
      buttonPFLight2->ProcessLoop(lastUpdateTime);
      buttonPFLight3->ProcessLoop(lastUpdateTime);
      buttonPFLight4->ProcessLoop(lastUpdateTime);
      buttonPFLight5->ProcessLoop(lastUpdateTime);
      buttonPFLight6->ProcessLoop(lastUpdateTime);

      // Increment the rest of the state machines.
      lightStation1->ProcessLoop(lastUpdateTime);
      lightStation2->ProcessLoop(lastUpdateTime);
      lightBugle1->ProcessLoop(lastUpdateTime);
      lightStorm1->ProcessLoop(lastUpdateTime);
      lightStorm2->ProcessLoop(lastUpdateTime);
  }

  if (useDisplay) {
      // Update the OLED screen with our current state.

      char headerBuffer[50];
      snprintf(headerBuffer, 50, "coolguybri cntrl 2.0");
      
      // Format the Uptime.
      int upSecs = (lastUpdateTime - startTime) / 1000;
      char line1Buffer[50];
      snprintf(line1Buffer, 50, "%09d %s", upSecs, buildDatestamp);
  
      // Windmill monitoring.
      char line2Buffer[50];
      snprintf(line2Buffer, 50, "wml: [%c] %s %03d %d%%", 
        windmillPower ? '*' : ' ', windmillDirection ? "<-" : "->", windmillVelocity, (windmillVelocity * 100) / 255);

      // Train monitoring
      char line3Buffer[50];
      snprintf(line3Buffer, 50, "trn: [%c] %s %03d %d%%", 
        trainPower ? '*' : ' ', trainDirection ? "<-" : "->", trainVelocity, (trainVelocity * 100) / 255);

      // PF-Light monitoring
      char line4Buffer[50];
      snprintf(line4Buffer, 50, "lit: [%c%c][%c][%c%c]", 
        lightStation1->IsOn() ? '*' : ' ', 
        lightStation2->IsOn() ? '*' : ' ',
        lightBugle1->IsOn() ? '*' : ' ', 
        lightStorm1->IsOn() ? '*' : ' ',
        lightStorm2->IsOn() ? '*' : ' ');
  
      // Send to the display.
      display.displayStandard(
        headerBuffer,
        line1Buffer,
        line2Buffer,
        line3Buffer, 
        line4Buffer);
  }
}
