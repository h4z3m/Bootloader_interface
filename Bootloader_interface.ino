#include "Bootloader_Host.h"
#include "Utilities.h"
Bootloader_Host* host;


void setup() {
	Serial.begin(9600);

	host = new Bootloader_Host();
	delay(10);
	uint8_t version = host->SendVersionCommand();

	if (version == 0x01) {
		DEBUG_PRINTF("Version = 0x%02X\n\r", (uint8_t)version);
	}

	host->SendFlashEraseCommand(0x08005000, 5);

	DEBUG_PRINTLN("Creating new array");

	uint8_t* data = new uint8_t[600];
	memset(data, 0x69, 600);
	DEBUG_PRINTLN("Created new array");

	host->SendMemWriteCommand(0x08005000, data, 600);

	host->SendJumpToAppCommand();
}

void loop() {

}