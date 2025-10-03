#ifndef __searob_springbutton_h__
#define __searob_springbutton_h__


/*
	Callback prototype for events triggered by the detected button press.
*/
class SeaRobSpringButton;
typedef void (*onPressDown) (SeaRobSpringButton *button, long updateTime);


/*
	Represents one physical button; when pressed, the onPressDown callback is invoked. 
	The physical button pops back up with a spring when released.
	Requires one input pin per button.
*/
class SeaRobSpringButton {
  public:
  				SeaRobSpringButton(String name, int pin, 
  							onPressDown downHandler, void *opaque = NULL);
  				
  		void 	ProcessLoop(unsigned long updateTime);
  		void * 	GetOpaque() { return _opaque; }
  		
  private:
	  String          _name;
	  int             _pin;
	  int             _levelPrev;
	  
	  onPressDown     _pressDownHandler;
	  void *          _opaque;
};


#endif // __searob_springbutton_h__
