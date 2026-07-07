#include "Arduino.h"
#include "MonorailSystem.h"
#include "SeaRobDisplay.h"
#include "SeaRobSpringButtonLight.h"
#include "SeaRobLogger.h"

// Constants: Specific I/O Pins that must be used.
// Assumes the Arduino Mega 3560 R3 Board.

// Constants: LCD Display subsystem
#define PIN_I2C_SDA                             20 // Dedicated SDA Output Pin (mega only)
#define PIN_I2C_SCL                             21 // Dedicated SCL Output Pin (mega only)

// Constants: Global/multi-slab
#define PIN_STREET_LIGHTS_BUTTON                26  // Digital Pin, input

// Contants: Slab5 subsytem
#define PIN_SLAB5_FE_A1_BUTTON                  28  // Digital Pin, input
#define PIN_SLAB5_FE_A1_CTRL                    51  // Digital Pin, output
#define PIN_SLAB5_FE_A2_BUTTON                  29  // Digital Pin, input
#define PIN_SLAB5_FE_A2_CTRL                    50  // Digital Pin, output
#define PIN_SLAB5_FE_A3_BUTTON                  30  // Digital Pin, input
#define PIN_SLAB5_FE_A3_CTRL                    49  // Digital Pin, output
#define PIN_SLAB5_FE_B1_BUTTON                  31  // Digital Pin, input
#define PIN_SLAB5_FE_B1_CTRL                    48  // Digital Pin, output
#define PIN_SLAB5_FE_B2_BUTTON                  32  // Digital Pin, input
#define PIN_SLAB5_FE_B2_CTRL                    47  // Digital Pin, output
#define PIN_SLAB5_FE_B3_BUTTON                  33  // Digital Pin, input
#define PIN_SLAB5_FE_B3_CTRL                    46  // Digital Pin, output
#define PIN_SLAB5_STREET_LIGHTS_CTRL            45  // Digital Pin, output

// Contants: Slab6 subsytem
#define PIN_SLAB6_CAVE_LIGHT_BUTTON             22  // Digital Pin, input
#define PIN_SLAB6_CAVE_LIGHT_CTRL               53  // Digital Pin, output
#define PIN_SLAB6_TRAINBRIDGE_REDBEAM_BUTTON    24  // Digital Pin, input
#define PIN_SLAB6_TRAINBRIDGE_REDBEAM_CTRL      8   // Digital Pin, PWM output
#define PIN_SLAB6_STREET_LIGHTS_CTRL            52  // Digital Pin, output

// Contants: Monorail subsytem
//#define PIN_MONORAIL_BUTTON     5
//#define MONORAIL_POLE_PIN_START_SLAB1     22

// Constants: Fade time for light effects
#define TRAINBRIDGE_REDBEAM_DURATION_ON 1000
#define TRAINBRIDGE_REDBEAM_DURATION_OFF 5000
#define TRAINBRIDGE_REDBEAM_DURATION_FADE 5000


// Global Variables: Global Run state of the entire aplication.
unsigned long   startTime = 0; 
unsigned long   lastUpdateTime = 0;
String          buildName = String("CascadiaSlabTown");
const char      buildTimestamp[] =  __DATE__ " " __TIME__;
const char      buildDatestamp[] = __DATE__;


// Global Variables: the OLED Display, connected via I2C interface
boolean useDisplay = true; 
SeaRobDisplay display(PIN_I2C_SDA, PIN_I2C_SCL);


// Global Variables: multi-slab
SeaRobSpringButtonLight *   streetLights = NULL;


// Global Variables: SLAB 5.
boolean useSlab5 =          true;
SeaRobSpringButtonLight *   feA1Light = NULL;
SeaRobSpringButtonLight *   feA2Light = NULL;
SeaRobSpringButtonLight *   feA3Light = NULL;
SeaRobSpringButtonLight *   feB1Light = NULL;
SeaRobSpringButtonLight *   feB2Light = NULL;
SeaRobSpringButtonLight *   feB3Light = NULL;


// Global Variables: SLAB 6.
boolean useSlab6 =          true;
SeaRobSpringButtonLight *   caveLight = NULL;
SeaRobSpringButtonLight *   trainBridgeRedBeamLight = NULL;


/* Global Variables: Monorail
boolean useSlab1 =      false;
MonorailSystem          monorail;
SeaRobSpringButton *    monorailButton;
*/


/*
 * I/O control callbacks from the control panel.
 */
 
/*int onMonorailButtonDown(SeaRobSpringButton *button, long updateTime) {
  bclogger("monorail light control: light mode toggle activated");
  monorail_system_state_increment(&monorail, updateTime);
}

 */

void onButtonDown_StreetLights(SeaRobSpringButtonLight *buttonLight, long updateTime) {
  bclogger("onButtonDown_StreetLights");
}

void onButtonDown_Slab5_FELight(SeaRobSpringButtonLight *buttonLight, long updateTime) {
  bclogger("onButtonDown_Slab5_FELight: %s", buttonLight->GetName());
}

void onButtonDown_Slab6_CaveLight(SeaRobSpringButtonLight *buttonLight, long updateTime) {
  bclogger("onButtonDown_Slab6_CaveLight");
}

void onButtonDown_Slab6_TrainBridge_RedBeamLight(SeaRobSpringButtonLight *buttonLight, long updateTime) {
  bclogger("onButtonDown_Slab6_TrainBridgeRedBeamLight");
}


/**
 * Entrypoint: called once when the program first starts, just to initialize all the sub-components.
 */
void setup() {  
  
  // Record what time we started.
  startTime = millis();
  
  // Init the serial line; important for debug messages back to the Arduino Serial Monitor..
  Serial.begin(9600);
  bclogger(""); // Skip line to seperate from last instance of the program.
  bclogger("setup: begin \"%s\" (build: %s)", buildName.c_str(), buildTimestamp);

  // Init the OLED display (for debug output and monitoring)
  if (useDisplay) {
    bclogger("setup: OLED start...");
    display.setup(buildTimestamp);
    bclogger("setup: OLED complete");
  }

  // Global streetlight button: various slabs will later add their pins to it.
  streetLights = new SeaRobSpringButtonLight("streetlights", 
        PIN_STREET_LIGHTS_BUTTON, -1, false, false, //non-dimmable, no internal pullup
        onButtonDown_StreetLights, NULL, NULL);

  // Init slab 5..
  if (useSlab5) {
    bclogger("setup: slab-5 start...");

    // Init the front-end passthrus in slab 5.
    feA1Light = new SeaRobSpringButtonLight("feA1", 
        PIN_SLAB5_FE_A1_BUTTON, PIN_SLAB5_FE_A1_CTRL, false, false, //non-dimmable, no internal pullup
        onButtonDown_Slab5_FELight, NULL, NULL);
    feA2Light = new SeaRobSpringButtonLight("feA2", 
        PIN_SLAB5_FE_A2_BUTTON, PIN_SLAB5_FE_A2_CTRL, false, false, //non-dimmable, no internal pullup
        onButtonDown_Slab5_FELight, NULL, NULL);
    feA3Light = new SeaRobSpringButtonLight("feA3", 
        PIN_SLAB5_FE_A3_BUTTON, PIN_SLAB5_FE_A3_CTRL, false, false, //non-dimmable, no internal pullup
        onButtonDown_Slab5_FELight, NULL, NULL);
    feB1Light = new SeaRobSpringButtonLight("feA1", 
        PIN_SLAB5_FE_B1_BUTTON, PIN_SLAB5_FE_B1_CTRL, false, false, //non-dimmable, no internal pullup
        onButtonDown_Slab5_FELight, NULL, NULL);
    feB2Light = new SeaRobSpringButtonLight("feA2", 
        PIN_SLAB5_FE_B2_BUTTON, PIN_SLAB5_FE_B2_CTRL, false, false, //non-dimmable, no internal pullup
        onButtonDown_Slab5_FELight, NULL, NULL);
    feB3Light = new SeaRobSpringButtonLight("feA3", 
        PIN_SLAB5_FE_B3_BUTTON, PIN_SLAB5_FE_B3_CTRL, false, false, //non-dimmable, no internal pullup
        onButtonDown_Slab5_FELight, NULL, NULL);

    // Wire up the slab5 section of the streelights to the global streetlight button.
    if (streetLights) {
      streetLights->AddExtraLedPin(PIN_SLAB5_STREET_LIGHTS_CTRL);
    }
    
    bclogger("setup: slab-5 complete.");
  }
  
  // Init slab 6, the far right slab.
  if (useSlab6) {
    bclogger("setup: slab-6 start...");

    // Cave Light - simple on/off light for the cave, no complex logic. Defaults to on at startup.
    caveLight = new SeaRobSpringButtonLight("slab6-cavelight", 
        PIN_SLAB6_CAVE_LIGHT_BUTTON, PIN_SLAB6_CAVE_LIGHT_CTRL, false, false, //non-dimmable, no internal pullup
        onButtonDown_Slab6_CaveLight, NULL, NULL); 
    caveLight->GetLight()->ToggleOnOff();

    // Train Bridge Light - cool 5v fader light in train bridge, set in continous loop. 
    // Turning on/off will just restart the cycle. Defaults to looping on/off at startup.
    trainBridgeRedBeamLight = new SeaRobSpringButtonLight("slab6-trainbidge-redbeam", 
        PIN_SLAB6_TRAINBRIDGE_REDBEAM_BUTTON, PIN_SLAB6_TRAINBRIDGE_REDBEAM_CTRL, true, false, // dimmable, needs pwm pin
        onButtonDown_Slab6_TrainBridge_RedBeamLight, NULL, NULL);
    trainBridgeRedBeamLight->GetLight()->UpdateBlinkConfig(0, 0, TRAINBRIDGE_REDBEAM_DURATION_ON, TRAINBRIDGE_REDBEAM_DURATION_OFF, 
        false, TRAINBRIDGE_REDBEAM_DURATION_FADE, TRAINBRIDGE_REDBEAM_DURATION_FADE);
    trainBridgeRedBeamLight->GetLight()->UpdateState(SeaRobLight::LightState::UniformBlink);

    // Wire up the slab6 section of the streelights to the global streetlight button.
    if (streetLights) {
      streetLights->AddExtraLedPin(PIN_SLAB6_STREET_LIGHTS_CTRL);
    }
    
    bclogger("setup: slab-6 complete.");
  }

  /*if (useSlab1) {
    monorail_system_setup(&monorail, MONORAIL_POLE_PIN_START_SLAB1);
    monorailButton = new SeaRobSpringButton("monorail light control", PIN_MONORAIL_BUTTON, &onMonorailButtonDown);
    bclogger("setup: slab-1 complete.");
  } */
  
  bclogger("setup: complete for \"%s\"", buildName.c_str());
}


/**
 * Main Loop: called over and over again as the robot runs.
 * Disopatches this event to all the subsystems.
 */
void loop() {

  // Get the current time.
  unsigned long now = lastUpdateTime = millis();

  if (streetLights) {
    streetLights->ProcessLoop(lastUpdateTime);
  }

  if (useSlab5) {
    if (feA1Light) 
      feA1Light->ProcessLoop(lastUpdateTime);
    if (feA2Light) 
      feA2Light->ProcessLoop(lastUpdateTime);
    if (feA3Light) 
      feA3Light->ProcessLoop(lastUpdateTime);
    if (feB1Light) 
      feB1Light->ProcessLoop(lastUpdateTime);
    if (feB2Light) 
      feB2Light->ProcessLoop(lastUpdateTime);
    if (feB3Light) 
      feB3Light->ProcessLoop(lastUpdateTime);
  }
  
  if (useSlab6) {
    if (caveLight)
      caveLight->ProcessLoop(lastUpdateTime);
    if (trainBridgeRedBeamLight)
      trainBridgeRedBeamLight->ProcessLoop(lastUpdateTime);
  }

  /*if (useSlab1) {
      // Process input first so they have immediate impact.
      monorailButton->ProcessLoop(lastUpdateTime);

      // Increment the rest of the state machines.
      monorail_system_loop(&monorail, lastUpdateTime);
  } */

  if (useDisplay) {
      // Update the OLED screen with our current state.

      #define LINE_BUFFER_SIZE 50
      char headerBuffer[LINE_BUFFER_SIZE];
      snprintf(headerBuffer, LINE_BUFFER_SIZE, "cascadia slabtown 1.0");
      
      // Format the Uptime.
      unsigned long upSecs = (lastUpdateTime - startTime) / 1000;
      char line1Buffer[LINE_BUFFER_SIZE];
      snprintf(line1Buffer, LINE_BUFFER_SIZE, "%09lu %s", upSecs, buildDatestamp);

       // TODO
      char line2Buffer[LINE_BUFFER_SIZE];
      snprintf(line2Buffer, 50, "");
          
      // FrontEnd-Light monitoring
      char line3Buffer[LINE_BUFFER_SIZE];
      strcpy(line3Buffer, "fe ");
      if (useSlab5) {
        int festrlen = strlen(line3Buffer);
        snprintf(line3Buffer + festrlen, LINE_BUFFER_SIZE - festrlen, " [%c%c%c%c%c%c]", 
          feA1Light && feA1Light->IsOn() ? '*' : 'o',
          feA2Light && feA2Light->IsOn() ? '*' : 'o',
          feA3Light && feA3Light->IsOn() ? '*' : 'o', 
          feB1Light && feB1Light->IsOn() ? '*' : 'o',
          feB2Light && feB2Light->IsOn() ? '*' : 'o',
          feB3Light && feB3Light->IsOn() ? '*' : 'o'); 
      }
        
      // PF-Light monitoring
      char line4Buffer[LINE_BUFFER_SIZE];
      strcpy(line4Buffer, "l ");
      if (useSlab6) {
        int litstrlen = strlen(line4Buffer);
        snprintf(line4Buffer + litstrlen, LINE_BUFFER_SIZE - litstrlen, " [%c%c%c]", 
          caveLight && caveLight->IsOn() ? '*' : 'o',
          trainBridgeRedBeamLight && trainBridgeRedBeamLight->IsOn() ? '*' : 'o',
          streetLights && streetLights->IsOn() ? '*' : 'o');
      }

      // Send to the display.
      display.displayStandard(
        headerBuffer,
        line1Buffer,
        line2Buffer,
        line3Buffer, 
        line4Buffer);
  }
}
