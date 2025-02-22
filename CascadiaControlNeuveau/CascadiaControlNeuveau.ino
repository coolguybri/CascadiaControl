#include "arduino.h"
#include "Logger.h"
#include "MonorailSystem.h"
#include "MotorPCM.h"
#include "SliderInput.h"
#include "SpringButton.h"

// Constants: Specific I/O Pins that must be used.
#define PIN_MONORAIL_BUTTON     5
#define PIN_CAVE_BUTTON         6

#define PIN_WINDMILL_BUTTON_1      5
#define PIN_WINDMILL_BUTTON_2      6
#define PIN_WINDMILL_BUTTON_3      7
#define PIN_WINDMILL_MOTOR_IN1     8
#define PIN_WINDMILL_MOTOR_IN2     9
#define PIN_WINDMILL_MOTOR_ENB     10

#define PIN_TRAIN_BUTTON_1      5
#define PIN_TRAIN_BUTTON_2      6
#define PIN_TRAIN_MOTOR_IN1     8
#define PIN_TRAIN_MOTOR_IN2     9
#define PIN_TRAIN_MOTOR_ENB     10

#define MONORAIL_POLE_PIN_START_SLAB1     22
#define CAVE_LIGHT_PIN                    52

#define PIN_I2C_SDA                     A4
#define PIN_I2C_SCL                     A5

// Global Variables: Global Run state of the entire aplication.
unsigned long startTime = 0; 
unsigned long lastUpdateTime = 0;
String buildName = String("BasementControl");
const char buildTimestamp[] =  __DATE__ " " __TIME__;

// Global Variables: the OLED Display, connected via I2C interface
//SeaRobDisplay display(PIN_I2C_SDA, PIN_I2C_SCL);

// Global Variables: StreetLight subsystem.
boolean useSlab1 = false;
MonorailSystem  monorail;
SpringButton    monorailButton;

boolean useSlab6 = false;
Light           caveLight;
SpringButton    caveButton;


// Windmill subsystem.
boolean       useWindmill = true;
boolean       windmillPower = false;
boolean       windmillDirection = true;
int           windmillVelocity = 120;
MotorPCM      motorWindmill;
SpringButton  buttonWindmill1;
SpringButton  buttonWindmill2;
SpringButton  buttonWindmill3;

// Train subsystem.
boolean       useTrain = true;
boolean       trainPower = false;
boolean       trainDirection = true;
int           trainVelocity = 120;
MotorPCM      motorTrain;
SpringButton  buttonTrain1;
SpringButton  buttonTrain2;

/*
 * Button callback
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
 * Windmill buttons
 */
 
int onButtonDownWindmill1(SpringButton *button, long updateTime) {
  windmillPower = !windmillPower;
  bclogger("windmill power: toggled to %d", windmillPower);

  if (windmillPower) {
    motor_set_state(&motorWindmill, MotorState_Forward);
  } else {
    motor_set_state(&motorWindmill, MotorState_Off);
  }
}

int onButtonDownWindmill2(SpringButton *button, long updateTime) {
  windmillVelocity += 20;
  bclogger("windmill up: %d", windmillVelocity);

  motor_set_pulsewidth(&motorWindmill, windmillVelocity);
}

int onButtonDownWindmill3(SpringButton *button, long updateTime) {
  windmillVelocity -= 20;
  bclogger("windmill down: %d", windmillVelocity);

  motor_set_pulsewidth(&motorWindmill, windmillVelocity);
}


/**
 * Entrypoint: called once when the program first starts, just to initialize all the sub-components.
 */
void setup() {  
  
  // Record what time we started.
  startTime = millis();
  
  // Init the serial line; important for debug messages back to the Arduino Serial Monitor.
  Serial.begin(9600);

  bclogger(""); // Skip line to seperate from last instance of the program.
  bclogger("setup: begin \"%s\" (build: %s)", buildName.c_str(), buildTimestamp);

  // Init the OLED display
  /*if (useDisplay) {
    display.setup(buildTimestamp);
    Serial.println(F("setup: OLED complete..."));
  } */

  if (useSlab1) {
    monorail_system_setup(&monorail, MONORAIL_POLE_PIN_START_SLAB1);
    springbutton_setup(&monorailButton, "monorail light control", PIN_MONORAIL_BUTTON, &onMonorailButtonDown);

    light_setup(&caveLight, CAVE_LIGHT_PIN, LightState::Off, 0);
    springbutton_setup(&caveButton, "cave light control", PIN_CAVE_BUTTON, &onCaveButtonDown);
    
    bclogger("setup: slab-1 complete.", 1);
  }

  if (useWindmill) {
    bclogger("setup: windmill start...");
    
    motor_setup(&motorWindmill, "windmill", PIN_WINDMILL_MOTOR_IN1, PIN_WINDMILL_MOTOR_IN2, PIN_WINDMILL_MOTOR_ENB);
    motor_set_pulsewidth(&motorWindmill, windmillVelocity); // middle value.
    
    springbutton_setup(&buttonWindmill1, "windmill power", PIN_WINDMILL_BUTTON_1, &onButtonDownWindmill1);
    springbutton_setup(&buttonWindmill2, "windmill speed up", PIN_WINDMILL_BUTTON_2, &onButtonDownWindmill2);
    springbutton_setup(&buttonWindmill3, "windmill speed down", PIN_WINDMILL_BUTTON_3, &onButtonDownWindmill3);

    bclogger("setup: windmill complete, power=%d, dir=%d, v=%d", windmillPower, windmillDirection, windmillVelocity);
  }

   if (useTrain) {
    bclogger("setup: train start...");
    
    motor_setup(&motorTrain, "train", PIN_TRAIN_MOTOR_IN1, PIN_TRAIN_MOTOR_IN2, PIN_TRAIN_MOTOR_ENB);
    motor_set_pulsewidth(&motorTrain, trainVelocity); // middle value.
    
    springbutton_setup(&buttonTrain1, "train power", PIN_TRAIN_BUTTON_1, &onButtonDownTrain1);
    springbutton_setup(&buttonTrain2, "train direction", PIN_TRAIN_BUTTON_2, &onButtonDownTrain2);

    bclogger("setup: train complete, power=%d, dir=%d, v=%d", trainPower, trainDirection, trainVelocity);
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

  // Process our input controls. 
  if (useSlab1) {
      // Process buttons first so they have immediate impact.
      springbutton_loop(&monorailButton, lastUpdateTime);
      springbutton_loop(&caveButton, lastUpdateTime);

      // Increment the rest of the state machines.
      monorail_system_loop(&monorail, lastUpdateTime);
      light_loop(&caveLight, lastUpdateTime);
  }

  if (useWindmill) {
      // Process buttons first so they have immediate impact.
      springbutton_loop(&buttonWindmill1, lastUpdateTime);
      springbutton_loop(&buttonWindmill2, lastUpdateTime);
      springbutton_loop(&buttonWindmill3, lastUpdateTime);

      // Increment the rest of the state machines.
      motor_loop(&motorWindmill, lastUpdateTime);
  }
}
