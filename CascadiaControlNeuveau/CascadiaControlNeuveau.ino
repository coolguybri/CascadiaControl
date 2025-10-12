#include "Arduino.h"
#include "SeaRobDisplay.h"
#include "SeaRobLight.h"
#include "SeaRobLogger.h"
#include "SeaRobSpringButton.h"
#include "SeaRobSpringButtonLightList.h"

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
#define PIN_PF_LIGHT_BUTTON_1       30 // Digital Pin, input  [30-34]
#define PIN_PF_LIGHT_CTRL_1         40 // Digital Pin, output [40-44]
#define PIN_PF_LIGHT_MODE_SELECTOR  35 // Digital Pin, output [35]

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
#define                         MAX_LIGHTS 5
boolean                         usePFLight = true;
SeaRobSpringButtonLightList *   buttonLightList = NULL;


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
    bclogger("setup: pf-light starting with maxLights=%d", MAX_LIGHTS);

    int buttonPins[] = { 30, 31, 32, 33, 34};
    int lightPins[] = { 40, 41, 44, 43, 42};
    buttonLightList = new SeaRobSpringButtonLightList(MAX_LIGHTS, buttonPins, lightPins, PIN_PF_LIGHT_MODE_SELECTOR);
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
      buttonLightList->ProcessLoop(lastUpdateTime);
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
      strcpy(line4Buffer, "lit: ");
      int litstrlen = strlen(line4Buffer);
      buttonLightList->GetStatusString(line4Buffer + litstrlen, 50 - litstrlen);
  
      // Send to the display.
      display.displayStandard(
        headerBuffer,
        line1Buffer,
        line2Buffer,
        line3Buffer, 
        line4Buffer);
  }
}
