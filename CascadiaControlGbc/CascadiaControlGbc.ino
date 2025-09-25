#include <SeaRobDisplay.h>

#include "arduino.h"
#include "Light.h"
#include "Logger.h"
#include "SpringButton.h"
#include "SeaRobDisplay.h"

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



// Global Variables: Global Run state of the entire aplication.
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
  BlinkState_SyncBlink,
  BlinkState_UnSyncBlink,
} BlinkState;

#define MAX_LIGHTS 8
#define DURATION_ON 500
#define DURATION_OFF 1000
class ButtonLight;

boolean         usePFLight = true;
ButtonLight *   buttonLights[MAX_LIGHTS];
SpringButton    buttonModeSelector;
BlinkState      bstate;


/*
 * 
 */
class ButtonLight {
  public:
    ButtonLight(int id, int buttonPin, int outputPin);

    void processLoop(long updateTime);

  public:
    static void OnButtonDown(SpringButton *button, long updateTime);
    
  public:
    int           index;
    Light         light;
    SpringButton  button;
};


/*
 */
ButtonLight::ButtonLight(int id, int buttonPin, int outputPin) {
  this->index = id;

  // Setup the light.
  light_setup(&(this->light), outputPin, LightState::Off, 0);

  // Setup the button.
  char buttonName[100];
  snprintf(buttonName, 100, "pf-light %02d", id);
  springbutton_setup(&button, buttonName, buttonPin, ButtonLight::OnButtonDown, this);
}


/*
 */
void ButtonLight::processLoop(long updateTime) {
  // Process button first (which could change our state if it were toggled).
  springbutton_loop(&button, updateTime);

  // Now handle the light (posibly switching it off or on).
  light_loop(&light, updateTime);
}


/*
 */
static void ButtonLight::OnButtonDown(SpringButton *button, long updateTime) {
  ButtonLight *bl = (ButtonLight *) button->_opaque;
  
  light_toggle_onoff(&(bl->light));
  bclogger("light %02d: toggled to %d", bl->index, bl->light.state);

  // Also turn off any blnking.
  bstate = BlinkState_Off;
}

   
/*
 * mode selector callbacks
 */

void onButtonDownLightIndividual(SpringButton *button, long updateTime) {
    //: invalidate the blinck mode.

  // Turn off any synchronized blinking if we are playing with individual buttons.
  bstate = BlinkState_Off;
}


void onButtonDownLightSelector(SpringButton *button, long updateTime) {

  // Incrmement through the states.
  switch (bstate) {
    case BlinkState_Off:
      bstate = BlinkState_SyncBlink;
      break;
      
    case BlinkState_SyncBlink:
      bstate = BlinkState_UnSyncBlink;
      break;
      
    case BlinkState_UnSyncBlink:
      bstate = BlinkState_Off;
      break;
  }  

  // Setup new state.
  switch (bstate) {
    case BlinkState_SyncBlink:
      for (int i = 0 ; i < MAX_LIGHTS ; i++) {
        ButtonLight * bl = buttonLights[i];
        light_update_blink(&(bl->light), updateTime, 0, DURATION_ON, DURATION_OFF);
        light_update_state(&(bl->light), LightState::UniformBlink);
      } 
      break;
      
    case BlinkState_UnSyncBlink:
      for (int i = 0 ; i < MAX_LIGHTS ; i++) {
        ButtonLight * bl = buttonLights[i];
        light_update_blink(&(bl->light), updateTime, (i * 125), DURATION_ON, DURATION_OFF);
        light_update_state(&(bl->light), LightState::UniformBlink);
      } 
      break;
      
    case BlinkState_Off:
      for (int i = 0 ; i < MAX_LIGHTS ; i++) {
        ButtonLight * bl = buttonLights[i];
        light_update_state(&(bl->light), LightState::Off);
      } 
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
      ButtonLight * bl = new ButtonLight(startId + i, startButtonPin + i, startOutputPin + i);
      buttonLights[i] = bl;
    }

    // Blink-mode selector button.
    springbutton_setup(&buttonModeSelector, "pf-light selector", PIN_PF_LIGHT_MODE_SELECTOR, &onButtonDownLightSelector, NULL);

    bclogger("setup: pf-light complete, mode=%d/%d/%d/%d/%d", 
      buttonLights[0]->light.state, buttonLights[1]->light.state, 
      buttonLights[2]->light.state, buttonLights[3]->light.state, 
      buttonLights[4]->light.state); 
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
      springbutton_loop(&buttonModeSelector, lastUpdateTime);
    
      for (int i = 0 ; i < MAX_LIGHTS ; i++) {
        buttonLights[i]->processLoop(lastUpdateTime);
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
     snprintf(line4Buffer, 50, "");
     snprintf(line4Buffer, 50, "lit: %c %c %c %c %c %c %c %c", 
        light_ison(&buttonLights[0]->light) ? '*' : '-', 
        light_ison(&buttonLights[1]->light) ? '*' : '-',
        light_ison(&buttonLights[2]->light) ? '*' : '-', 
        light_ison(&buttonLights[3]->light) ? '*' : '-',
        light_ison(&buttonLights[4]->light) ? '*' : '-', 
        light_ison(&buttonLights[5]->light) ? '*' : '-',
        light_ison(&buttonLights[6]->light) ? '*' : '-', 
        light_ison(&buttonLights[7]->light) ? '*' : '-');
  
      // Send to the display.
      display.displayStandard(
        headerBuffer,
        line1Buffer,
        line2Buffer,
        line3Buffer, 
        line4Buffer); 
  }
}
