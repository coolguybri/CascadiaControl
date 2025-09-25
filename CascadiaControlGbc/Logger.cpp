#include "arduino.h"
#include "Logger.h"

/*
 * Global log buffer.
 */
#define LOGBUF_SIZE 500
char logbuf[LOGBUF_SIZE];


/*
 * Global logger; currently outputs to the Serial Monitor.
 */
void bclogger(const char* fmt, ...) {
    va_list params;
    va_start(params, fmt);

    vsnprintf(logbuf, 500, fmt, params);
    Serial.println(logbuf);

    va_end(params);
}
