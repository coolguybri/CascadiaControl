#include "Arduino.h"
#include "SeaRobObject.h"
#include "SeaRobLogger.h"


/* static class object (global) */
int SeaRobObject::s_nextId = 0;


/*
 */
SeaRobObject::SeaRobObject(): _objId(++s_nextId), _creationTime(millis()) {
  bclogger("SeaRobObject: ctor: objId=%d, creation=%lu", _objId, _creationTime);
}


/*
 */
SeaRobObject::~SeaRobObject() {
	unsigned long now = millis();
	bclogger("SeaRobObject: dtor: objId=%d, deletion=%lu, elapsed=%lu", _objId, now, (now - _creationTime));
}

