// Import external libraries
#include "MonorailSystem.h"

// TODO: cascadia code; move to seperate file.
#define PIN_MONORAIL_BUTTON     5
#define PIN_CAVE_BUTTON         6
#define MONORAIL_POLE_PIN_START_SLAB1     22
#define CAVE_LIGHT_PIN                    52


// Constants: Specific I/O Pins that must be used.
#define PIN_SLAB6_STREETLIGHT_BUTTON     5
#define PIN_SLAB6_STREETLIGHT_1          9
#define PIN_SLAB6_STREETLIGHT_2         10
#define PIN_SLAB6_STREETLIGHT_3         11
#define PIN_SLAB6_STREETLIGHT_4         12
#define PIN_I2C_SDA                     A4
#define PIN_I2C_SCL                     A5


// Data Sructures: enum for the state of the lights.
typedef enum {
  Off = 0,
  Solid,
  UniformBlink,
} StreetLightState;


// Global Variables: Feature Enablement.
//  To temporarily disable various subsystems, set these appropriately.
boolean useSlab6 = true;

// Global Variables: Global Run state of the entire aplication.
unsigned long startTime = 0; 
const char buildTimestamp[] =  __DATE__ " " __TIME__;

// Global Variables: the OLED Display, connected via I2C interface
//SeaRobDisplay display(PIN_I2C_SDA, PIN_I2C_SCL);


// Global Variables: StreetLight subsystem.
StreetLightState  _streetLightState;
unsigned long     _streetLightStateStartTime;
int               _streetLightButtonPin;
boolean           _streetLightButtonLevelCurr;
boolean           _streetLightButtonLevelPrev;


// Global Variables: old cascadia subsystem. TODO: move to new file.
boolean useSlab1 =      false;
MonorailSystem          monorail;
SeaRobSpringButton *    monorailButton;
SeaRobLight *           caveLight;
SeaRobSpringButton *    caveButton;

/*
 * Cascadia callback.
 */
 
int onMonorailButtonDown(SeaRobSpringButton *button, long updateTime) {
  bclogger("monorail light control: light mode toggle activated");
  monorail_system_state_increment(&monorail, updateTime);
}

int onCaveButtonDown(SeaRobSpringButton *button, long updateTime) {
  bclogger("cave light control: light mode toggle activated");
  caveLight->ToggleOnOff();
}


/**
 * Entrypoint: called once when the program first starts, just to initialize all the sub-components.
 */
void setup() {  
  
  // Record what time we started.
  startTime = millis();
  
  // Init the serial line; important for debug messages back to the Arduino Serial Monitor..
  Serial.begin(9600);
  Serial.println(F("setup: begin..."));

  if (useSlab1) {
    monorail_system_setup(&monorail, MONORAIL_POLE_PIN_START_SLAB1);
    monorailButton = new SeaRobSpringButton("monorail light control", PIN_MONORAIL_BUTTON, &onMonorailButtonDown);

    caveLight = new SeaRobLight(CAVE_LIGHT_PIN);
    caveButton = new SeaRobSpringButton("cave light control", PIN_CAVE_BUTTON, &onCaveButtonDown);
    
    bclogger("setup: slab-1 complete.", 1);
  }
  
  if (useSlab6) {
    streetLightsSetup();
    Serial.println(F("setup: slab-6 complete..."));
  }
  
  // Init the OLED display
  /*if (useDisplay) {
    display.setup(buildTimestamp);
    Serial.println(F("setup: OLED complete..."));
  } */

  // Init the rest of our internal state.
  Serial.println(F("setup: end"));
}


/**
 * Main Loop: called over and over again as the robot runs.
 * Disopatches this event to all the subsystems.
 */
void loop() {

  // Get the current time.
  unsigned long now = millis();

  // Process our input controls. 
  if (useSlab1) {
      // Process input first so they have immediate impact.
      monorailButton->ProcessLoop(lastUpdateTime);
      caveButton->ProcessLoop(lastUpdateTime);

      // Increment the rest of the state machines.
      monorail_system_loop(&monorail, lastUpdateTime);
      caveLight->ProcessLoop(lastUpdateTime);
  }
  
  // Process our input controls. 
  if (useSlab6) {
    streetLightsLoop(now);
  }
}


/**
 * 
 */
void streetLightsSetup() {

  // Init headlight input control pin to the button.
  _streetLightButtonPin = PIN_SLAB6_STREETLIGHT_BUTTON;
  _streetLightButtonLevelCurr = _streetLightButtonLevelPrev = LOW;
  pinMode(_streetLightButtonPin, INPUT_PULLUP);

  // Init LED output control pins to the lights.
  pinMode(PIN_SLAB6_STREETLIGHT_1, OUTPUT);
  pinMode(PIN_SLAB6_STREETLIGHT_2, OUTPUT);
  pinMode(PIN_SLAB6_STREETLIGHT_3, OUTPUT);
  pinMode(PIN_SLAB6_STREETLIGHT_4, OUTPUT);

  // Set the initial state: On solid.
  _streetLightState = StreetLightState::Solid;
  _streetLightStateStartTime = 0;
  _streetLightStateStartTimeInit = true;
}


/*
 * Called once per update to monitor the state of the street light toggle button.
 */
void streetLightsLoop(unsigned long now) {

  // Light Button Control: Detect if the voltage level on the button has changed.
  _streetLightButtonLevelCurr = digitalRead(_streetLightButtonPin);  
  if (_streetLightButtonLevelCurr != _streetLightButtonLevelPrev) {

    // If this is a button-down event, trigger a state change for the street light subsystem.
    if (_streetLightButtonLevelCurr == HIGH) {
      streetLightsStateIncrement(now);
    }

    _streetLightButtonLevelPrev = _streetLightButtonLevelCurr;
  }

  // Light State Machine: increment the state machine of any of the light states.
  streetLightsStateLoop(now);
}


/*
 * Called every streetlight control button-down.
 */
void streetLightsStateIncrement(unsigned long now) {

  // Cycle through the available states.
  switch (_streetLightState) {
  case StreetLightState::Off:
    _streetLightState = StreetLightState::Solid;
  case StreetLightState::Solid:
   // _streetLightState = StreetLightState::UniformBlink;  
  //case StreetLightState::UniformBlink:
    _streetLightState = StreetLightState::Off;
  }

  // Record start time of state machine.
  _streetLightStateStartTime = now;
 // _streetLightStateStartTimeInit = true;

  Serial.print(F("Headlight state now: "));
  Serial.println(_streetLightState);
}


/*
 * Called every loop.
 */
void streetLightsStateLoop(unsigned long now) {

  // Cycle through the available states.
  switch (_streetLightState) {
    
  case StreetLightState::Off:
  case StreetLightState::Solid: {
      bool level = (_streetLightState == StreetLightState::Solid);
      digitalWrite(PIN_SLAB6_STREETLIGHT_1, level);
      digitalWrite(PIN_SLAB6_STREETLIGHT_2, level);
      digitalWrite(PIN_SLAB6_STREETLIGHT_3, level);
      digitalWrite(PIN_SLAB6_STREETLIGHT_4, level);
    }
    break;
    
  case StreetLightState::UniformBlink:
      // TODO: implement blinking algorithm. 
   
  }


}
