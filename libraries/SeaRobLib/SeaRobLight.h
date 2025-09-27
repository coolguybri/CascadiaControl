#ifndef __searob_light_h__
#define __searob_light_h__

/*
 * Represents one led that can be either on or off. One output pin is required per light. 
 *  It can be set to blink overtime, or just stay in its current state until set again.
 */
class SeaRobLight {

  public:
  	typedef enum {
  	  Off = 0,
  	  On,
  	  UniformBlink,
  	} LightState;

  public:
  					SeaRobLight(int pin, int offset=0);
  					
  		void		UpdateState(LightState state);
  		void		UpdateBlinkConfig(unsigned long startTime, int offset, int durationOn, int durationOff);
  		void		ToggleOnOff();
      	void    	SetDebugLogging(bool setter);
  					
  		bool		IsOn();
  		String		GetStateName();
  					
  		void		ProcessLoop(unsigned long updateTime);
  							
  private:
	  const int        pin;
	  
	  LightState       state;
	  int              blinkOffset;
	  int              blinkIntervalOn;
	  int              blinkIntervalOff;
	
	  unsigned long    blinkTimeNext;
	  bool             litState;
      bool             loggingState;
};

#endif // __searob_light_h__
