#ifndef CONFIG_H
#define CONFIG_H

#include "Arduino.h"
#include <Preferences.h>
Preferences prefs;

#define max(a,b) ((a)>(b)?(a):(b)) 
#define min(a,b) ((a)<(b)?(a):(b))

#define NOTE_PIN 26
#define MIC_PIN 23
#define ENABLED 0 
#define DISABLED 1

const String OTA_HOST = "ffe80891.ngrok.io"; //project-518.herokuapp.com
const String OTA_SKETCH = "/sketch.ino.bin";
const String OTA_VERSION = "/version";

#endif
