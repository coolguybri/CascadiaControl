#ifndef __searob_springbuttonlightlist_h__
#define __searob_springbuttonlightlist_h__

#include "SeaRobSpringButtonLight.h"


/*
	Represents the pairing of one button to one LED; the button will auto switch it on and off.
*/
class SeaRobSpringButtonLightList {

  public:
	typedef enum {
	  BlinkState_Off = 0,
	  BlinkState_ConstantOn,
	  BlinkState_SyncBlink,
	  BlinkState_UnSyncBlink,
	  BlinkState_UnSyncBlinkSlow,
	} BlinkState;

  public:
    		SeaRobSpringButtonLightList(int numLights, int startButtonPin, int startLightPin, int selectorButtonPin);
    		SeaRobSpringButtonLightList(int numLights, int* buttonPins, int* lightPins, int selectorButtonPin);
    		virtual ~SeaRobSpringButtonLightList();

    void 	ProcessLoop(unsigned long updateTime);
    void	GetStatusString(char *buf, int buflen);
    
  protected:
	  void OnButtonDownLightSelector(long updateTime);
	  void OnButtonDownLightIndividual(long updateTime);
	  void HandleStateChange(long updateTime);
	  
  protected:
		static void StaticOnButtonDownLightIndividual(SeaRobSpringButtonLight *buttonLight, long updateTime) {
		 	SeaRobSpringButtonLightList *bll = (SeaRobSpringButtonLightList *) buttonLight->GetOpaque(); 
			bll->OnButtonDownLightIndividual(updateTime);
		}
		
		static void StaticOnButtonDownLightSelector(SeaRobSpringButton *button, long updateTime) {
			SeaRobSpringButtonLightList *bll = (SeaRobSpringButtonLightList *) button->GetOpaque(); 
			bll->OnButtonDownLightSelector(updateTime);
		}

  private:
  	const int							_numLights;
  	const SeaRobSpringButtonLight **   	_buttonLights;
  	const SeaRobSpringButton *        	_buttonModeSelector;
	BlinkState      		        	_blinkState;
};


#endif // __searob_springbuttonlightlist_h__
