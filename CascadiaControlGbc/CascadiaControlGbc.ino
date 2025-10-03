#include "Arduino.h"
#include "SeaRobDisplay.h"
#include "SeaRobLight.h"
#include "SeaRobLogger.h"
#include "SeaRobSpringButton.h"
#include "SeaRobSpringButtonLightList.h"

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
#define                         MAX_LIGHTS 6
boolean         		            usePFLight = true;
SeaRobSpringButtonLightList *   buttonLightList = NULL;

   
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
	  buttonLightList = new SeaRobSpringButtonLightList(MAX_LIGHTS, PIN_PF_LIGHT_BUTTON_1, PIN_PF_LIGHT_CTRL_1, PIN_PF_LIGHT_MODE_SELECTOR);
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
    buttonLightList->ProcessLoop(lastUpdateTime);
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
      strcat(line4Buffer, "lit: ");
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
