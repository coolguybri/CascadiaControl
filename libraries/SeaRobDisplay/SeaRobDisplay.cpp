// Import external libraries
#include "SeaRobDisplay.h"

/**
 * Called at global static init time
 */
SeaRobDisplay::SeaRobDisplay(int pinSda, int pinScl) : 
  _display(pinSda, pinScl),
  _timestamp(""),
  _bluetoothSet(false),
  _bluetoothName("unknown"),
  _bluetoothAddr("unknown") {
}
  
/**
 * Called at setup() time.
 */  
void SeaRobDisplay::setup(const char *timestamp) {
  // Save this for later, so we can print it out on the display.
  _timestamp = timestamp;
  
  // This is called at app setup time, and through tinkering have discovered that a slight delay
  // prevents flakiness.
  delay(1000);
  _display.initialize();
}

/**
 * Called after startup, after hardware fully initializes.
 */
void SeaRobDisplay::setupBluetoothName(
      	const char *bluetoothName,
      	const char *bluetoothAddr) {
  _bluetoothName = bluetoothName;
  _bluetoothAddr = bluetoothAddr;
  _bluetoothSet = true;
}


/**
 * Main output for status while in main sequence. This is called once per loop.
 */
void SeaRobDisplay::displayConnectedStandard(
    		  boolean 		connected, 
    		  const char * 	line1,
    		  const char * 	line2, 
    		  const char * 	line3) {
  displayStandard(connected ? "CONNECTED" : "DISCONNECTED", line1, line2, line3, NULL);
}
    		  
/**
 * Main output for status while in main sequence. This is called once per loop.
 */
void SeaRobDisplay::displayStandard(const char *header, const char *line1, const char *line2, 
			const char *line3, const char *line4) { 
  // Init for output. 
  _display.clear();
  _display.setTextSize(1);
  _display.setTextColor(WHITE);

  // Fun little animation to prove that we are not locked up.
  const int circleRadius = 5;
  const int circleOffset = 0;
  const int maxRight = (SCREEN_WIDTH - 1) - circleRadius;
  const int maxLeft = 66;
  int numFrames = maxRight - maxLeft;
  int numFramesDouble = numFrames * 2;
  int timePerFrame = 3000 / numFramesDouble;
  int currentFrame = (millis() / timePerFrame) % numFramesDouble;
  int circleCenterY = circleRadius + circleOffset;
  int circleCenterX = (currentFrame < numFrames) ? (maxRight - currentFrame) : (maxLeft + (currentFrame - numFrames));
  _display.drawCircle(circleCenterX, circleCenterY, circleRadius, WHITE);
  
  // Finish Header.
  _display.setCursor(0,1);
  _display.println(header);
  _display.drawLine(0, 13, SCREEN_WIDTH - 1, 13, WHITE); // top line
  
   // Print the custom text lines.
  _display.setCursor(0,17);
  _display.println(line1);
  _display.setCursor(0,27);
  _display.println(line2);
  _display.setCursor(0,37);
  _display.println(line3);
  _display.setCursor(0,47);
  _display.println(line4);
  
  /* Print the local bluetooth name.
  int beginBlueName = 36;
  _display.setCursor(0, beginBlueName);
  _display.print(F("name:"));
  _display.setCursor(34, beginBlueName);
  _display.println(_bluetoothName); */

  /* Print the remote bluetooth address.
  int beginBlue = 46;
  _display.setCursor(0, beginBlue);
  _display.print("addr:");
  _display.setCursor(34, beginBlue);
  _display.println(_bluetoothAddr); */
  
  // Print the timestamp of when we built this code.
  //const int stampHeight = 10;
  //int beginStamp = 55;
  //_display.drawLine(0, beginStamp, SCREEN_WIDTH - 1, beginStamp, WHITE); // top line
  //_display.drawLine(0, beginStamp, 0, beginStamp + stampHeight, WHITE); // left line
  //_display.drawLine(SCREEN_WIDTH - 1, beginStamp, SCREEN_WIDTH - 1, beginStamp + stampHeight, WHITE); // right line
  //_display.setCursor(4, beginStamp + 2);
  //_display.println(_timestamp);

  _display.update();
}