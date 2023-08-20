#include <HardwareSerial.h>
#include <Arduino.h>
#include <memory>
#include "Bootloader_Host.h"
#include "BootloaderCommand.h"
#include "Utilities.h"
#include "bl/inc/bl_cmd_types.h"


Bootloader_Host::Bootloader_Host() {
	pinMode(LED, OUTPUT);
	digitalWrite(LED, HIGH);
	myPort.begin(9600, SWSERIAL_8N1, MYPORT_RX, MYPORT_TX, false);
	if (!myPort) { // If the object did not initialize, then its configuration is invalid
		Serial.println("Error initializing software serial");
		while (1);
	}
	delay(100);
	if (!SendEnterCmdModeCommand())
	{
		Serial.println("Error entering command mode");
	}
	delay(100);

}

void Bootloader_Host::blinkLED(int duration) {
	digitalWrite(LED, LOW);
	delay(duration);
	digitalWrite(LED, HIGH);
}

void Bootloader_Host::printHeader(BL_CommandHeader_t& header) {
	DEBUG_PRINTF("Command ID = 0x%02X", (uint8_t)header.cmd_id);
	DEBUG_PRINTF("Payload size = 0x%08X", (uint32_t)header.payload_size);
	DEBUG_PRINTF("CRC32 = 0x%08X", (uint32_t)header.CRC32);
}

void Bootloader_Host::printCommand(void* cmd, BL_CommandID_t id) {

	switch (id) {
	case BL_GOTO_ADDR_CMD_ID:
		DEBUG_PRINTLN("**** GO TO ADDR CMD ****");
		printHeader(static_cast<BL_GOTO_ADDR_CMD*>(cmd)->data.header);
		DEBUG_PRINTF("Address = 0x%08X\n\r", (uint32_t)static_cast<BL_GOTO_ADDR_CMD*>(cmd)->data.address);
		break;
	case BL_MEM_WRITE_CMD_ID:
		DEBUG_PRINTLN("**** MEM WRITE CMD ****");
		printHeader(static_cast<BL_MEM_WRITE_CMD*>(cmd)->data.header);
		break;
	case BL_MEM_READ_CMD_ID:
		DEBUG_PRINTLN("**** MEM READ CMD ****");
		printHeader(static_cast<BL_MEM_READ_CMD*>(cmd)->data.header);
		DEBUG_PRINTF("Start address = 0x%08X", static_cast<BL_MEM_READ_CMD*>(cmd)->data.start_addr);
		DEBUG_PRINTF("Length = 0x%08X", static_cast<BL_MEM_READ_CMD*>(cmd)->data.length);
		break;
	case BL_VER_CMD_ID:
		DEBUG_PRINTLN("**** VER CMD ****");
		printHeader(static_cast<BL_VER_CMD*>(cmd)->data.header);
		break;
	case BL_FLASH_ERASE_CMD_ID:
		DEBUG_PRINTLN("**** FLASH ERASE CMD ****");
		printHeader(static_cast<BL_FLASH_ERASE_CMD*>(cmd)->data.header);
		DEBUG_PRINTF("Start address = 0x%08X", static_cast<BL_FLASH_ERASE_CMD*>(cmd)->data.page_number);
		DEBUG_PRINTF("Count = 0x%08X", static_cast<BL_FLASH_ERASE_CMD*>(cmd)->data.page_count);
		break;
	case BL_ACK_CMD_ID:
		DEBUG_PRINTLN("**** ACK CMD ****");
		DEBUG_PRINTF("Command ID = 0x%02X", (uint8_t)static_cast<BL_ACK*>(cmd)->data.cmd_id);
		DEBUG_PRINTF("ACK = 0x%02X", (uint8_t) static_cast<BL_ACK*>(cmd)->data.ack);
		DEBUG_PRINTF("NACK field = 0x%02X", (uint8_t)static_cast<BL_ACK*>(cmd)->data.field);
		break;
	case BL_DATA_PACKET_CMD_ID:
		DEBUG_PRINTLN("**** DATA PACKET CMD ****");
		printHeader(static_cast<BL_DATA_PACKET_CMD*>(cmd)->data.header);
		DEBUG_PRINTF("Data length = 0x%08X", static_cast<BL_DATA_PACKET_CMD*>(cmd)->data.data_len);
		DEBUG_PRINTF("Next block length = 0x%08X", static_cast<BL_DATA_PACKET_CMD*>(cmd)->data.next_len);
		DEBUG_PRINTF("End flag = 0x%02X", (uint8_t) static_cast<BL_DATA_PACKET_CMD*>(cmd)->data.end_flag);
		DEBUG_PRINTLN("Data block (first 20 bytes) = ");
		for (uint32_t i = 0; i < 20; ++i) {
			Serial.printf("0x%02X ", (uint8_t) static_cast<BL_DATA_PACKET_CMD*>(cmd)->data.data_block[i]);
		}
		Serial.println();
		break;
	case BL_JUMP_TO_APP_CMD_ID:
		DEBUG_PRINTLN("**** JUMP TO APP CMD ****");
		printHeader(static_cast<BL_JUMP_TO_APP_CMD*>(cmd)->data.header);
		DEBUG_PRINTF("Key = 0x%08X", static_cast<BL_JUMP_TO_APP_CMD*>(cmd)->data.key);
		break;
	case BL_ENTER_CMD_MODE_CMD_ID:
		DEBUG_PRINTLN("**** ENTER CMD MODE CMD ****");
		printHeader(static_cast<BL_ENTER_CMD_MODE_CMD*>(cmd)->data.header);
		DEBUG_PRINTF("Key = 0x%08X", static_cast<BL_ENTER_CMD_MODE_CMD*>(cmd)->data.key);
		break;
	default:
		DEBUG_PRINTLN("Unknown command ID");
		break;
	}
}

uint8_t Bootloader_Host::SendVersionCommand() {
	std::unique_ptr<BL_VER_CMD> cmd = CreateVerCommand();
	if (!cmd.get())
		return 0;

	printCommand(cmd.get(), BL_VER_CMD_ID);
	SendCommand(cmd.get()->serialized_data, sizeof(BL_VER_CMD));

	uint8_t nack_field = 0xFF;
	bool ack_received = ReceiveAck(&nack_field);

	if (!ack_received)
		return 0;

	uint32_t response_length = 0;
	bool response_received = ReceiveResponse(&response_length);

	if (!response_received)
		return 0;

	BL_Response* rsp = (BL_Response*)(rx_buffer);

	// Validate response
	uint32_t test_crc = bl_calculate_command_crc(rsp, sizeof(BL_Response));
	if (!VALIDATE_CMD(rsp->serialized_data, rsp->data.header.payload_size,
		rsp->data.header.CRC32))
	{
		DEBUG_PRINTF("Invalid CRC %08X", rsp->data.header.CRC32);
		DEBUG_PRINTF("Calculated CRC %08X", test_crc);
		return 0;
	}

	return rsp->data.data[0];
}

bool Bootloader_Host::SendFlashEraseCommand(uint32_t page_start_address, uint32_t page_count)
{
	std::unique_ptr<BL_FLASH_ERASE_CMD> cmd = CreateFlashEraseCommand(page_start_address, page_count);
	if (!cmd.get())
		return false;

	printCommand(cmd.get(), BL_FLASH_ERASE_CMD_ID);
	SendCommand(cmd.get()->serialized_data, sizeof(BL_FLASH_ERASE_CMD));

	uint8_t nack_field = 0xFF;
	bool ack_received = ReceiveAck(&nack_field);

	if (!ack_received)
		return false;

	ack_received = ReceiveAck(&nack_field);

	if (!ack_received)
		return false;

	return true;
}

uint8_t Bootloader_Host::SendMemWriteCommand(uint32_t start_address, uint8_t data[], uint32_t data_size) {
	uint32_t number_of_blocks = data_size / 256;
	uint32_t remainder_bytes = data_size % 256;

	DEBUG_PRINTF("Number of blocks to send = %d", number_of_blocks);
	DEBUG_PRINTF("Remainder bytes = %d", remainder_bytes);

	std::unique_ptr<BL_MEM_WRITE_CMD> cmd = CreateMemWriteCommand(start_address);
	if (!cmd.get())
		return 0;

	printCommand(cmd.get(), BL_MEM_WRITE_CMD_ID);
	SendCommand(cmd.get()->serialized_data, sizeof(BL_MEM_WRITE_CMD));

	/* Wait for ack on command */
	uint8_t nack_field = 0xFF;
	bool ack_received = ReceiveAck(&nack_field);

	if (!ack_received)
		return 0;

	/* To store the next block size to send */
	uint32_t next_block = 256;

	/* Proceed to send the rest of the data */
	for (size_t i = 0; i < number_of_blocks; i++)
	{
		/* If this is the last even block, assign next block size to remainder bytes size */
		if (i == number_of_blocks - 1)
			next_block = remainder_bytes;


		std::unique_ptr<BL_DATA_PACKET_CMD> block = CreateDataPacketCommand(&data[256U * i], 256, next_block,
			((i + 1) * 256U) == data_size);

		printCommand(block.get(), BL_DATA_PACKET_CMD_ID);

		if (!block.get())
			return 0;

		SendCommand(block.get()->serialized_data, block.get()->data.header.payload_size);

		/* Wait for ack on last packet*/
		uint8_t nack_field = 0xFF;
		bool ack_received = ReceiveAck(&nack_field);

		if (!ack_received)
			return 0;
	}

	if (remainder_bytes)
	{
		std::unique_ptr<BL_DATA_PACKET_CMD> block = CreateDataPacketCommand(&data[256U * number_of_blocks], remainder_bytes, 0,
			true);

		if (!block.get())
			return 0;

		printCommand(block.get(), BL_DATA_PACKET_CMD_ID);
		SendCommand(block.get()->serialized_data, block.get()->data.header.payload_size);

		/* Wait for ack on last packet*/
		uint8_t nack_field = 0xFF;
		bool ack_received = ReceiveAck(&nack_field);

		if (!ack_received)
			return 0;
	}

	delay(10);
	return 1;

}

bool Bootloader_Host::SendEnterCmdModeCommand() {
	std::unique_ptr<BL_ENTER_CMD_MODE_CMD> cmd = CreateEnterCmdModeCommand(ENTER_CMD_MODE_KEY);
	if (!cmd.get())
		return false;

	printCommand(cmd.get(), BL_ENTER_CMD_MODE_CMD_ID);
	SendCommand(cmd.get()->serialized_data, sizeof(BL_ENTER_CMD_MODE_CMD));

	uint8_t nack_field = 0xFF;
	bool ack_received = ReceiveAck(&nack_field);

	if (!ack_received)
		return false;

	return true;
}

bool Bootloader_Host::SendJumpToAppCommand() {

	std::unique_ptr<BL_JUMP_TO_APP_CMD> cmd = CreateJumpToAppCommand(JUMP_APP_KEY);
	if (!cmd.get())
		return false;

	printCommand(cmd.get(), BL_JUMP_TO_APP_CMD_ID);
	SendCommand(cmd.get()->serialized_data, sizeof(BL_JUMP_TO_APP_CMD));

	uint8_t nack_field = 0xFF;
	bool ack_received = ReceiveAck(&nack_field);

	if (!ack_received)
		return false;

	return true;
}

void Bootloader_Host::SendCommand(uint8_t* data, uint32_t bytes) {
	if (state != HostState::ReadyToSendCommand) {
		SyncClient();
	}
	myPort.write(data, bytes);
}

bool Bootloader_Host::ReceiveResponse(uint32_t* length) {
	int i = 0;

	/* Wait for any data to arrive */
	while (myPort.available() == 0);

	*length = myPort.available();
	myPort.readBytes(rx_buffer, sizeof(BL_CommandHeader_t));
	myPort.readBytes(&rx_buffer[sizeof(BL_CommandHeader_t)], ((BL_CommandHeader_t*)(rx_buffer))->payload_size);

	blinkLED(100);

	return true;
}

bool Bootloader_Host::ReceiveAck(uint8_t* nack_field) {

	/* Wait for any data to arrive */
	while (myPort.available() == 0);

	BL_ACK ack = { 0 };

	myPort.readBytes(ack.serialized_data, sizeof(ack.serialized_data));

	if (ack.data.field != 0xFF && nack_field)
		*nack_field = ack.data.field;

	printCommand(&ack, BL_ACK_CMD_ID);

	if (ack.data.ack)
		blinkLED(100);

	return (ack.data.ack == 1);
}

void Bootloader_Host::SyncClient() {
	uint8_t temp = 0;

	// Continuosly read from serial if received sync byte
	while (temp != SYNC_BYTE) {
		if (myPort.available())
		{
			myPort.read(&temp, 1);
			// Exit loop so we don't send next sync byte
			if (temp == SYNC_BYTE)
				break;
		}
		/* Send the sync byte then wait for a response */
		myPort.write((uint8_t*)&SYNC_BYTE, 1);
		delay(1000);
	}

	/* Synchronization successful */
	state = HostState::ReadyToSendCommand;

}

Bootloader_Host::~Bootloader_Host() {

}
