#include "Arduino.h"
#include "SeaRobDisplay.h"
#include "SeaRobLight.h"
#include "SeaRobLogger.h"
#include "SeaRobSpringButton.h"
#include "SeaRobSpringButtonLight.h"

// Constants: Specific I/O Pins that must be used.
// Assumes the Arduino Mega 3560 R3 Board.

// Lego PowerFunctions Light Array: button input pins
#define PIN_PF_LIGHT_MODE_SELECTOR  22 // Digital Pin, input
#define PIN_PF_LIGHT_BUTTON_1       24 // Digital Pin, input

// Lego PowerFunctions Light Array: output state pins
#define PIN_PF_LIGHT_CTRL_1         40 // Digital Pin, output

// LCD Display subsystem
#define PIN_I2C_SDA                 20 // Dedicated SDA Output Pin (mega only)
#define PIN_I2C_SCL                 21 // Dedicated SCL Output Pin (mega only)


// Global Variables: Global Run state of the entire application.
unsigned long   startTime = 0; 
unsigned long   lastUpdateTime = 0;
String          buildName = String("GbcControl2");
const char      buildTimestamp[] =  __DATE__ " " __TIME__;
const char      buildDatestamp[] = __DATE__;


// Globals: OLED Display subsystem, connected via I2C interface
boolean useDisplay = true; 
SeaRobDisplay display(PIN_I2C_SDA, PIN_I2C_SCL);

// Globals: PowerFunctions (PF) Lights

typedef enum {
  BlinkState_Off = 0,
  BlinkState_ConstantOn,
  BlinkState_SyncBlink,
  BlinkState_UnSyncBlink,
} BlinkState;

#define MAX_LIGHTS 6
#define DURATION_ON 500
#define DURATION_OFF 1000

boolean         		        usePFLight = true;
SeaRobSpringButtonLight *   buttonLights[MAX_LIGHTS];
SeaRobSpringButton *        buttonModeSelector = NULL;
BlinkState      		        bstate;

   
/*
 * mode selector callbacks
 */

void onButtonDownLightIndividual(SeaRobSpringButtonLight *bl, long updateTime) {
  // Turn off any synchronized blinking if we are playing with individual buttons.
  if (bstate != BlinkState_Off) {
    bstate = BlinkState_Off;
    handleStateChange(updateTime);
  }
}


void onButtonDownLightSelector(SeaRobSpringButton *button, long updateTime) {

  // Incrmement through the states.
  switch (bstate) {
    case BlinkState_Off:
      bstate = BlinkState_ConstantOn;
      break;

    case BlinkState_ConstantOn:
      bstate = BlinkState_SyncBlink;
      break;
      
    case BlinkState_SyncBlink:
      bstate = BlinkState_UnSyncBlink;
      break;
      
    case BlinkState_UnSyncBlink:
      bstate = BlinkState_Off;
      break;
  }  

  handleStateChange(updateTime);
}


/*
 */
void handleStateChange(long updateTime) {

  // Setup new state.
  switch (bstate) {
    case BlinkState_SyncBlink:
      for (int i = 0 ; i < MAX_LIGHTS ; i++) {
        SeaRobSpringButtonLight * bl = buttonLights[i];
        bl->GetLight()->UpdateBlinkConfig(updateTime, 0, DURATION_ON, DURATION_OFF);
        bl->GetLight()->UpdateState(SeaRobLight::LightState::UniformBlink);
      } 
      break;
      
    case BlinkState_UnSyncBlink: {
        int perLightDelay = 200;
        int durationOn = 2000;
        int durationOff = 500;
        for (int i = 0 ; i < MAX_LIGHTS ; i++) {
          SeaRobSpringButtonLight * bl = buttonLights[i];
          bl->GetLight()->UpdateBlinkConfig(updateTime, (i * perLightDelay), durationOn, durationOff);
          bl->GetLight()->UpdateState(SeaRobLight::LightState::UniformBlink);
        } 
    }
      break;
      
    case BlinkState_Off:
      for (int i = 0 ; i < MAX_LIGHTS ; i++) {
        SeaRobSpringButtonLight * bl = buttonLights[i];
        bl->GetLight()->UpdateState(SeaRobLight::LightState::Off);
      } 
      break;

    case BlinkState_ConstantOn:
      for (int i = 0 ; i < MAX_LIGHTS ; i++) {
        SeaRobSpringButtonLight * bl = buttonLights[i];
        bl->GetLight()->UpdateState(SeaRobLight::LightState::On);
      } 
      break;

    default:
      bclogger("lightSelector: ERROR: undefined state: %d", bstate);
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

  if (usePFLight) {
    bclogger("setup: pf-light starting with maxLights=%d", MAX_LIGHTS);

    // Init the light array.
    int startId = 1;
    int startButtonPin = PIN_PF_LIGHT_BUTTON_1;
    int startOutputPin = PIN_PF_LIGHT_CTRL_1;
    for (int i = 0 ; i < MAX_LIGHTS ; i++) {
      SeaRobSpringButtonLight * bl = new SeaRobSpringButtonLight(startId + i, startButtonPin + i, startOutputPin + i, onButtonDownLightIndividual);
      buttonLights[i] = bl;
    }

    // Blink-mode selector button.
    buttonModeSelector = new SeaRobSpringButton("pf-light selector", PIN_PF_LIGHT_MODE_SELECTOR, &onButtonDownLightSelector, NULL);

    bclogger("setup: pf-light complete, mode=%d/%d/%d/%d/%d", 
      buttonLights[0]->IsOn(), 
      buttonLights[1]->IsOn(), 
      buttonLights[2]->IsOn(), 
      buttonLights[3]->IsOn(), 
      buttonLights[4]->IsOn()); 
  }
  
  bclogger("setup: complete for \"%s\"", buildName.c_str());
}


/**
 * Main Loop: called over and over again as the robot runs.
 * Disopatches this event to all the subsystems.
 */
void loop() {

  // Get the current time.
  lastUpdateTime = millis();
  //bclogger("loop: called with \"%00d\"", lastUpdateTime);

  if (usePFLight) {
      buttonModeSelector->ProcessLoop(lastUpdateTime);
    
      for (int i = 0 ; i < MAX_LIGHTS ; i++) {
        buttonLights[i]->ProcessLoop(lastUpdateTime);
      }
  } 

  if (useDisplay) {
      // Update the OLED screen with our current state.

      char headerBuffer[50];
      snprintf(headerBuffer, 50, "connolly brother ctrl");
      
      // Format the Uptime.
      int upSecs = (lastUpdateTime - startTime) / 1000;
      char line1Buffer[50];
      snprintf(line1Buffer, 50, "%09d %s", upSecs, buildDatestamp);

      char line2Buffer[50];
      snprintf(line2Buffer, 50, "");
      
      char line3Buffer[50];
      snprintf(line3Buffer, 50, "");

      // PF-Light monitoring
      char line4Buffer[50];
      snprintf(line4Buffer, 50, "lit: %c %c %c %c %c %c %c %c", 
        buttonLights[0]->IsOn() ? '*' : '-', 
        buttonLights[1]->IsOn() ? '*' : '-',
        buttonLights[2]->IsOn() ? '*' : '-', 
        buttonLights[3]->IsOn() ? '*' : '-',
        buttonLights[4]->IsOn() ? '*' : '-', 
        buttonLights[5]->IsOn() ? '*' : '-',
        buttonLights[6]->IsOn() ? '*' : '-', 
        buttonLights[7]->IsOn() ? '*' : '-');
  
      // Send to the display.
      display.displayStandard(
        headerBuffer,
        line1Buffer,
        line2Buffer,
        line3Buffer, 
        line4Buffer); 
  }
}
