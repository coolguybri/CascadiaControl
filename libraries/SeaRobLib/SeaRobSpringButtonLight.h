#ifndef __searob_springbuttonlight_h__
#define __searob_springbuttonlight_h__

#include "SeaRobSpringButton.h"
#include "SeaRobLight.h"


/*
	Callback prototype for events triggered by the detected button press.
*/
class SeaRobSpringButtonLight;
typedef void (*onStateChange) (SeaRobSpringButtonLight *bl, long updateTime);


/*
	Represents the pairing of one button to one LED; the button will auto switch it on and off.
*/
class SeaRobSpringButtonLight {

  public:
    		SeaRobSpringButtonLight(int uniqueId, int buttonPin, int ledPin, onStateChange stateChangeHandler);
    		virtual ~SeaRobSpringButtonLight();

    void 	ProcessLoop(unsigned long updateTime);
    
     SeaRobLight *  GetLight() { return _light; }
     SeaRobSpringButton * GetButton() { return _button; }
     bool IsOn() { return _light->IsOn(); }

  protected:
    void 	OnButtonDown(long updateTime);
    
  private:
    int                   _label;
    SeaRobLight *         _light;
    SeaRobSpringButton *  _button;
    onStateChange    	  _stateChangeHandler;
    
  protected:
	static void StaticOnButtonDown(SeaRobSpringButton *button, long updateTime) {
	  SeaRobSpringButtonLight *bl = (SeaRobSpringButtonLight *) button->GetOpaque();
	  bl->OnButtonDown(updateTime);
	}
};


#endif // __searob_springbuttonlight_h__
