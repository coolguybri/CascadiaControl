#include "arduino.h"
#include "Logger.h"
#include "MonorailSystem.h"
#include "MotorPCM.h"
#include "SliderInput.h"
#include "SpringButton.h"
#include "SeaRobDisplay.h"

// Constants: Specific I/O Pins that must be used.
// Assumes the Arduino Mega 3560 R3 Board.

// Windmill Subsystem
#define PIN_WINDMILL_BUTTON_PWR     22 // Digital Pin, input
#define PIN_WINDMILL_BUTTON_DIR     23 // Digital Pin, input
#define PIN_WINDMILL_BUTTON_INC     24 // Digital Pin, input
#define PIN_WINDMILL_BUTTON_DEC     25 // Digital Pin, input
#define PIN_WINDMILL_MOTOR_IN1      32 // Digital Pin, output
#define PIN_WINDMILL_MOTOR_IN2      33 // Digital Pin, output
#define PIN_WINDMILL_MOTOR_ENB      2  // Digital & PWM Output Pin

// Train subsystem
#define PIN_TRAIN_BUTTON_PWR        26 // Digital Pin, input
#define PIN_TRAIN_BUTTON_DIR        27 // Digital Pin, input
#define PIN_TRAIN_BUTTON_INC        28 // Digital Pin, input
#define PIN_TRAIN_BUTTON_DEC        29 // Digital Pin, input
#define PIN_TRAIN_SLIDER            A1 // Analog Input Pin
#define PIN_TRAIN_MOTOR_IN1         34 // Digital Pin, output
#define PIN_TRAIN_MOTOR_IN2         35 // Digital Pin, output
#define PIN_TRAIN_MOTOR_ENB         3  // Digital & PWM Output Pin

// LCD Display subsystem
#define PIN_I2C_SDA               20 // Dedicated SDA Output Pin (mega only)
#define PIN_I2C_SCL               21 // Dedicated SCL Output Pin (mega only)

// TBD: cascadia code; move tp seperate file.
#define PIN_MONORAIL_BUTTON     5
#define PIN_CAVE_BUTTON         6
#define MONORAIL_POLE_PIN_START_SLAB1     22
#define CAVE_LIGHT_PIN                    52


// Global Variables: Global Run state of the entire aplication.
unsigned long startTime = 0; 
unsigned long lastUpdateTime = 0;
String buildName = String("BasementControl");
const char buildTimestamp[] =  __DATE__ " " __TIME__;
const char buildDatestamp[] = __DATE__;

// Globals: OLED Display subsystem, connected via I2C interface
boolean useDisplay = true; 
SeaRobDisplay display(PIN_I2C_SDA, PIN_I2C_SCL);

// Globals: Windmill subsystem.
boolean       useWindmill = true;
boolean       windmillPower = false;
boolean       windmillDirection = true;
int           windmillVelocity = 120;
MotorPCM      motorWindmill;
SpringButton  buttonWindmillPwr;
SpringButton  buttonWindmillDir;
SpringButton  buttonWindmillInc;
SpringButton  buttonWindmillDec;

// Globals: Train subsystem.
boolean       useTrain = true;
boolean       trainPower = false;
boolean       trainDirection = true;
int           trainVelocity = 200;
MotorPCM      motorTrain;
SpringButton  buttonTrainPwr;
SpringButton  buttonTrainDir;
SpringButton  buttonTrainInc;
SpringButton  buttonTrainDec;
SliderInput   sliderTrain;

// Global Variables: old cascadia subsystem. TODO: move to new file.
boolean useSlab1 = false;
MonorailSystem  monorail;
SpringButton    monorailButton;
boolean useSlab6 = false;
Light           caveLight;
SpringButton    caveButton;


/*
 * Cascadia callback. TODO: move to a new file.
 */
 
int onMonorailButtonDown(SpringButton *button, long updateTime) {
  bclogger("monorail light control: light mode toggle activated");
  monorail_system_state_increment(&monorail, updateTime);
}

int onCaveButtonDown(SpringButton *button, long updateTime) {
  bclogger("cave light control: light mode toggle activated");
  light_toggle_onoff(&caveLight);
}


/*
 * Windmill button callbacks
 */
 
void onButtonDownWindmillPwr(SpringButton *button, long updateTime) {
  windmillPower = !windmillPower;
  bclogger("windmill power: toggled to %d", windmillPower);

  if (windmillPower) {
    motor_set_state(&motorWindmill, windmillDirection ? MotorState_Forward : MotorState_Reverse);
  } else {
    motor_set_state(&motorWindmill, MotorState_Off);
  }
}

void onButtonDownWindmillDir(SpringButton *button, long updateTime) {
  windmillDirection = !windmillDirection;
  bclogger("windmill direction: toggled to %d", windmillDirection);

  if (windmillPower) {
    motor_set_state(&motorWindmill, windmillDirection ? MotorState_Forward : MotorState_Reverse);
  } else {
    motor_set_state(&motorWindmill, MotorState_Off);
  }
}

void onButtonDownWindmillInc(SpringButton *button, long updateTime) {
  windmillVelocity += 20;
  windmillVelocity = motor_set_pulsewidth(&motorWindmill, windmillVelocity);
  bclogger("windmill inc: %d", windmillVelocity);
}

void onButtonDownWindmillDec(SpringButton *button, long updateTime) {
  windmillVelocity -= 20;
  windmillVelocity = motor_set_pulsewidth(&motorWindmill, windmillVelocity);
  bclogger("windmill dec: %d", windmillVelocity);
}


/*
 * Train button callbacks
 */
 
void onButtonDownTrainPwr(SpringButton *button, long updateTime) {
  trainPower = !trainPower;
  bclogger("train power: toggled to %d", trainPower);

  if (trainPower) {
    motor_set_state(&motorTrain, trainDirection ? MotorState_Forward : MotorState_Reverse);
  } else {
    motor_set_state(&motorTrain, MotorState_Off);
  }
}

void onButtonDownTrainDir(SpringButton *button, long updateTime) {
  trainDirection = !trainDirection;
  bclogger("train direction: toggled to %d", trainDirection);

  if (trainPower) {
    motor_set_state(&motorTrain, trainDirection ? MotorState_Forward : MotorState_Reverse);
  } else {
    motor_set_state(&motorTrain, MotorState_Off);
  }
}

int onButtonDownTrainInc(SpringButton *button, long updateTime) {
  trainVelocity += 20;
  trainVelocity = motor_set_pulsewidth(&motorTrain, trainVelocity);
  bclogger("train inc: %d", trainVelocity);
}

int onButtonDownTrainDec(SpringButton *button, long updateTime) {
  trainVelocity -= 20;
  trainVelocity = motor_set_pulsewidth(&motorTrain, trainVelocity);
  bclogger("train dec: %d", trainVelocity);
}


int onSliderChangeTrain(SliderInput *input, int newValue, long updateTime) {
  //trainDirection = !trainDirection;
  bclogger("train velocity: set to %d", newValue);

  // TODO: translate from 104 ratio to 2256 raio. Store as float?
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
    
    springbutton_setup(&buttonWindmillPwr, "windmill power", PIN_WINDMILL_BUTTON_PWR, &onButtonDownWindmillPwr);
    springbutton_setup(&buttonWindmillDir, "windmill direction", PIN_WINDMILL_BUTTON_DIR, &onButtonDownWindmillDir);
    springbutton_setup(&buttonWindmillInc, "windmill speed inc", PIN_WINDMILL_BUTTON_INC, &onButtonDownWindmillInc);
    springbutton_setup(&buttonWindmillDec, "windmill speed dec", PIN_WINDMILL_BUTTON_DEC, &onButtonDownWindmillDec);

    bclogger("setup: windmill complete, power=%d, dir=%d, v=%d", windmillPower, windmillDirection, windmillVelocity);
  }

  if (useTrain) {
    bclogger("setup: train start...");
    
    motor_setup(&motorTrain, "train", PIN_TRAIN_MOTOR_IN1, PIN_TRAIN_MOTOR_IN2, PIN_TRAIN_MOTOR_ENB);
    trainVelocity = motor_set_pulsewidth(&motorTrain, trainVelocity); // middle value.
    
    springbutton_setup(&buttonTrainPwr, "train power", PIN_TRAIN_BUTTON_PWR, &onButtonDownTrainPwr);
    springbutton_setup(&buttonTrainDir, "train direction", PIN_TRAIN_BUTTON_DIR, &onButtonDownTrainDir);
    springbutton_setup(&buttonTrainInc, "train up", PIN_TRAIN_BUTTON_INC, &onButtonDownTrainInc);
    springbutton_setup(&buttonTrainDec, "train down", PIN_TRAIN_BUTTON_DEC, &onButtonDownTrainDec);
    //sliderinput_setup(&sliderTrain, "train velocity", PIN_TRAIN_SLIDE, &onSliderChangeTrain);

    bclogger("setup: train complete, power=%d, dir=%d, v=%d", trainPower, trainDirection, trainVelocity);
  }

  // TODO: move this to a different file.
  if (useSlab1) {
    monorail_system_setup(&monorail, MONORAIL_POLE_PIN_START_SLAB1);
    springbutton_setup(&monorailButton, "monorail light control", PIN_MONORAIL_BUTTON, &onMonorailButtonDown);

    light_setup(&caveLight, CAVE_LIGHT_PIN, LightState::Off, 0);
    springbutton_setup(&caveButton, "cave light control", PIN_CAVE_BUTTON, &onCaveButtonDown);
    
    bclogger("setup: slab-1 complete.", 1);
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
      springbutton_loop(&buttonWindmillPwr, lastUpdateTime);
      springbutton_loop(&buttonWindmillDir, lastUpdateTime);
      springbutton_loop(&buttonWindmillInc, lastUpdateTime);
      springbutton_loop(&buttonWindmillDec, lastUpdateTime);
    
      // Increment the rest of the state machines.
      motor_loop(&motorWindmill, lastUpdateTime);
  }

  if (useTrain) {
      // Process inputs first so they have immediate impact.
      springbutton_loop(&buttonTrainPwr, lastUpdateTime);
      springbutton_loop(&buttonTrainDir, lastUpdateTime);
      springbutton_loop(&buttonTrainInc, lastUpdateTime);
      springbutton_loop(&buttonTrainDec, lastUpdateTime);
      //sliderinput_loop(&sliderTrain, lastUpdateTime);

      // Increment the rest of the state machines.
      motor_loop(&motorTrain, lastUpdateTime);
  }

  if (useDisplay) {
      // Update the OLED screen with our current state.

      char headerBuffer[50];
      snprintf(headerBuffer, 50, "coolguybri cntrl 1.0");
      
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
  
      // Send to the display.
      display.displayStandard(
        headerBuffer,
        line1Buffer,
        line2Buffer,
        line3Buffer);
  }

  // Process our input controls. 
  // TODO: move this to another file.
  if (useSlab1) {
      // Process input first so they have immediate impact.
      springbutton_loop(&monorailButton, lastUpdateTime);
      springbutton_loop(&caveButton, lastUpdateTime);

      // Increment the rest of the state machines.
      monorail_system_loop(&monorail, lastUpdateTime);
      light_loop(&caveLight, lastUpdateTime);
  }
}
