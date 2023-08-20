#pragma once
#include <stdint.h>
#include <SoftwareSerial.h>
#include "bl/inc/bl_cmd_types.h"

#define LED (D0)
#define MYPORT_TX 12
#define MYPORT_RX 14

class Bootloader_Host
{
	enum class HostState
	{
		Synchronization,
		SendingData,
		ReadyToSendCommand,
		WaitingForAck
	};
	const int SYNC_BYTE = 0xA5;
	const uint32_t JUMP_APP_KEY = 0x4032AFE5;
	const uint32_t ENTER_CMD_MODE_KEY = 0x09B21FFC;


	SoftwareSerial myPort;
	HostState state = HostState::Synchronization;
	uint8_t rx_buffer[256];
public:
	Bootloader_Host();

	uint8_t SendVersionCommand();
	bool SendFlashEraseCommand(uint32_t page_start_address, uint32_t page_count);
	uint8_t SendMemWriteCommand(uint32_t start_address, uint8_t data[], uint32_t data_size);
	bool SendEnterCmdModeCommand();
	bool SendJumpToAppCommand();
	~Bootloader_Host();

private:
	void printHeader(BL_CommandHeader_t& header);
	void printCommand(void* cmd, BL_CommandID_t id);
	void blinkLED(int duration = 1000);
	bool ReceiveResponse(uint32_t* length);
	bool ReceiveAck(uint8_t* nack_field);
	bool SendAck(void);
	void SyncClient(void);
	void SendCommand(uint8_t* data, uint32_t bytes);
};
