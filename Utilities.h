#pragma once
#include <Arduino.h>
#define DEBUG_PRINTLN(str)\
do{\
Serial.println(str );	\
} while (0);\


#define DEBUG_PRINTF(format,...)					\
do{													\
Serial.printf(format"\n\r", __VA_ARGS__);	\
} while (0);										\
