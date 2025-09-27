#include "Arduino.h"
#include "SeaRobLogger.h"

/*
 * Global log buffer.
 */
#define LOGBUF_SIZE 500
char logbuf[LOGBUF_SIZE + 1];


/*
 * Global logger; currently outputs to the Serial Monitor.
 */
void bclogger(const char* fmt, ...) {
    va_list params;
    va_start(params, fmt);

    vsnprintf(logbuf, LOGBUF_SIZE, fmt, params);
    Serial.println(logbuf);

    va_end(params);
}
