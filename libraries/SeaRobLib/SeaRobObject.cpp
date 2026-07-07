#include "Arduino.h"
#include "SeaRobObject.h"
#include "SeaRobLogger.h"


/* static class object (global) */
int SeaRobObject::s_nextId = 0;


/*
 */
SeaRobObject::SeaRobObject(): _objId(++s_nextId), _creationTime(millis()) {
  bclogger("SeaRobObject [%d]: ctor: creation=%lu", _objId, _creationTime);
}


/*
 */
SeaRobObject::~SeaRobObject() {
	unsigned long now = millis();
	bclogger("SeaRobObject [%d]: dtor: creation=%lu, deletion=%lu, lifetime=%lu", 
		_objId, _creationTime, now, (now - _creationTime));
}
