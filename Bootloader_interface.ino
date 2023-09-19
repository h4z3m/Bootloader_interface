#include <base64.hpp>
#include "Bootloader_Host.h"
#include "Utilities.h"
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ESP_WiFiManager.h>
#include <ESP_WiFiManager.hpp>
#include <WebSocketsClient_Generic.h>
#include <base64.hpp>

Bootloader_Host* host;

// WiFi credentials
const char* ssid = "Hazem";
const char* password = "98765432102";

ESP_WiFiManager wifiManager;

// WebSocket server details
const char* serverAddress = "172.20.10.2";
//const char* serverAddress = "192.168.1.17";
const int serverPort = 8080;

// WebSocket client object
WebSocketsClient webSocket;

void handleVersionEvent()
{
	uint8_t version = host->SendVersionCommand();

	StaticJsonDocument<128> versionJsonBuffer;
	versionJsonBuffer["error"] = host->last_nack_fields;
	versionJsonBuffer["version"] = version;
	versionJsonBuffer["commandId"] = BL_VER_CMD_ID;
	versionJsonBuffer["status"] = (version != 0);
	String jsonData;
	serializeJson(versionJsonBuffer, jsonData);
	webSocket.sendTXT(jsonData);
}

void handleFlashEraseEvent(uint32_t address, uint32_t count)
{
	bool status = host->SendFlashEraseCommand(address, count);

	StaticJsonDocument<128> eraseJsonBuffer;
	eraseJsonBuffer["commandId"] = BL_FLASH_ERASE_CMD_ID;
	eraseJsonBuffer["status"] = status;
	eraseJsonBuffer["error"] = host->last_nack_fields;

	String jsonData;
	serializeJson(eraseJsonBuffer, jsonData);
	webSocket.sendTXT(jsonData);
}

void handleMemoryWriteEvent(uint32_t start_address, uint8_t binary_data[], uint32_t size)
{
	bool status = host->SendMemWriteCommand(start_address, binary_data, size);

	StaticJsonDocument<128> memoryWriteJsonBuffer;
	memoryWriteJsonBuffer["commandId"] = BL_MEM_WRITE_CMD_ID;
	memoryWriteJsonBuffer["status"] = status;
	memoryWriteJsonBuffer["error"] = host->last_nack_fields;

	String jsonData;
	serializeJson(memoryWriteJsonBuffer, jsonData);
	webSocket.sendTXT(jsonData);
}

void handleMemoryReadEvent(uint32_t start_address, uint32_t length) {

	if (length >= ESP.getFreeHeap()) {
		DynamicJsonDocument memoryReadJsonBuffer = DynamicJsonDocument(128);
		String jsonData;
		memoryReadJsonBuffer["commandId"] = BL_MEM_READ_CMD_ID;
		memoryReadJsonBuffer["status"] = false;
		memoryReadJsonBuffer["error"] = 0;
		serializeJson(memoryReadJsonBuffer, jsonData);
		webSocket.sendTXT(jsonData);
	}
	else {
		uint8_t* buffer = new uint8_t[length];
		bool status = host->SendMemReadCommand(start_address, length, buffer);

		DynamicJsonDocument memoryReadJsonBuffer = DynamicJsonDocument(length + 128);
		memoryReadJsonBuffer["commandId"] = BL_MEM_READ_CMD_ID;
		memoryReadJsonBuffer["status"] = status;
		memoryReadJsonBuffer["error"] = host->last_nack_fields;
		JsonArray binary = memoryReadJsonBuffer.createNestedArray("binaryData");

		for (int i = 0; i < length; i++) {
			binary.add(buffer[i]);
		}
		String jsonData;

		serializeJson(memoryReadJsonBuffer, jsonData);
		webSocket.sendTXT(jsonData);

		memoryReadJsonBuffer.clear();
		delete buffer;
	}
}

void handleJumpToAppCommand()
{
	bool status = host->SendJumpToAppCommand();

	StaticJsonDocument<128> jumpAppJsonBuffer;
	jumpAppJsonBuffer["commandId"] = BL_JUMP_TO_APP_CMD_ID;
	jumpAppJsonBuffer["status"] = status;
	jumpAppJsonBuffer["error"] = host->last_nack_fields;

	String jsonData;
	serializeJson(jumpAppJsonBuffer, jsonData);
	webSocket.sendTXT(jsonData);
}

// Callback function when WebSocket connection is established
void webSocketEvent(WStype_t type, uint8_t* payload, size_t length)
{

	switch (type)
	{
	case WStype_DISCONNECTED:
		DEBUG_PRINTLN("WebSocket disconnected");
		break;
	case WStype_CONNECTED:
		DEBUG_PRINTLN("WebSocket connected");
		break;
	case WStype_TEXT:
	{
		DEBUG_PRINTF("Received message of length: %d", length);

		DynamicJsonDocument jsonBuffer(128);
		DEBUG_PRINTF("Size of json buffer: %d", jsonBuffer.capacity());
		DeserializationError error = deserializeJson(jsonBuffer, (char*)payload);
		if (error)
		{
			DEBUG_PRINTF("Error parsing JSON: %s", error.c_str());
			return;
		}

		BL_CommandID_t command = jsonBuffer["commandId"];
		DEBUG_PRINTF("Command: %d", command);

		switch (command)
		{
		case BL_VER_CMD_ID:
			DEBUG_PRINTLN(F("Version command"));
			handleVersionEvent();
			break;
		case BL_FLASH_ERASE_CMD_ID:
		{
			DEBUG_PRINTLN(F("Flash erase command"));
			uint32_t address = jsonBuffer["address"];
			uint32_t count = jsonBuffer["count"];
			DEBUG_PRINTF("Flash address = %08X, count = %08X", address, count);
			handleFlashEraseEvent(address, count);
		}
		break;
		case BL_MEM_WRITE_CMD_ID:
		{
			DEBUG_PRINTLN(F("Memory write command"));
			uint32_t address = jsonBuffer["address"];
			uint32_t size = jsonBuffer["size"];
			const char* binaryFile = jsonBuffer["binaryData"];
			uint8_t* decoded = new uint8_t[size];
			unsigned int out_size = decode_base64((unsigned char*)binaryFile, (unsigned char*)decoded);
			handleMemoryWriteEvent(address, (uint8_t*)decoded, size);
			delete decoded;
		}
		break;

		case BL_JUMP_TO_APP_CMD_ID:
		{
			DEBUG_PRINTLN(F("Jump to app command"));
			handleJumpToAppCommand();
		}
		break;
		case BL_MEM_READ_CMD_ID:
		{
			DEBUG_PRINTLN(F("Memory read command"));
			uint32_t address = jsonBuffer["address"];
			uint32_t length = jsonBuffer["length"];
			handleMemoryReadEvent(address, length);
		}
		break;
		default:
			DEBUG_PRINTLN(F("Unknown text event"));
			break;
		}
	}
	break;
	case WStype_BIN:
		DEBUG_PRINTF("[WSc] get binary length: %u\n", length);
	}
}

void initializeBootloader()
{
	host = Bootloader_Host::getInstance();
	delay(100);

	uint8_t version = host->SendVersionCommand();

	if (version == 0x01)
	{
		DEBUG_PRINTF("Version = 0x%02X\n\r", (uint8_t)version);
	}
	else {
		DEBUG_PRINTF("Failed to verify version, got v.%d", version);
	}
	//EEPROM.begin(16000);
}

void setup()
{
	Serial.begin(9600);
	DEBUG_PRINTF("Max heap size at boot = %d\n", ESP.getFreeHeap());
	// Reset WiFi settings and start WiFiManager configuration portal
	//wifiManager.resetSettings();
	DEBUG_PRINTLN(F("Starting autoconnect"));
	WiFi.enableInsecureWEP();
	WiFi.mode(WIFI_STA);
	WiFi.hostname("ESP-host");
	//wifiManager.autoConnect();
	DEBUG_PRINTLN(F("Initializing bootloader"));
	initializeBootloader();

	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(1000);
		DEBUG_PRINTLN(F("Connecting to WiFi..."));
	}
	DEBUG_PRINTF("Connected to WiFi with IP: %s", WiFi.localIP().toString());
	// Set up WebSocket event handler
	webSocket.begin(serverAddress, serverPort, "/");
	webSocket.onEvent(webSocketEvent);
}

void loop()
{
	webSocket.loop();
}
