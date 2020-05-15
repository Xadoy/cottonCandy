#ifndef HEADER_UTILITIES
#define HEADER_UTILITIES


/**
 * Helper function for timing. On Arduino, it can be done using millis().
 */
unsigned long getTimeMillis();


/**
 * Helper function for sleeping. On Arduino, it can be done using delay().
 */
void sleepForMillis(unsigned long time);

#endif