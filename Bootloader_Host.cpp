#include <HardwareSerial.h>
#include <Arduino.h>
#include <memory>
#include "Bootloader_Host.h"
#include "BootloaderCommand.h"
#include "Utilities.h"
#include "bl_utils.h"
#include "bl_cmd_types.h"

Bootloader_Host* Bootloader_Host::instance = nullptr;

Bootloader_Host* Bootloader_Host::getInstance() {
	if (instance == nullptr) instance = new Bootloader_Host();
	return instance;
}

Bootloader_Host::Bootloader_Host() {
	pinMode(LED, OUTPUT);
	digitalWrite(LED, HIGH);
	myPort.begin(9600, SWSERIAL_8N1, MYPORT_RX, MYPORT_TX, false);
	if (!myPort) { // If the object did not initialize, then its configuration is invalid
		Serial.println("Error initializing software serial");
		while (1);
	}
	myPort.setTimeout(1000);

	//delay(100);
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
		DEBUG_PRINTLN(F("**** GO TO ADDR CMD ****"));
		printHeader(static_cast<BL_GOTO_ADDR_CMD*>(cmd)->data.header);
		DEBUG_PRINTF("Address = 0x%08X\n\r", (uint32_t)static_cast<BL_GOTO_ADDR_CMD*>(cmd)->data.address);
		break;
	case BL_MEM_WRITE_CMD_ID:
		DEBUG_PRINTLN(F("**** MEM WRITE CMD ****"));
		printHeader(static_cast<BL_MEM_WRITE_CMD*>(cmd)->data.header);
		break;
	case BL_MEM_READ_CMD_ID:
		DEBUG_PRINTLN(F("**** MEM READ CMD ****"));
		printHeader(static_cast<BL_MEM_READ_CMD*>(cmd)->data.header);
		DEBUG_PRINTF("Start address = 0x%08X", static_cast<BL_MEM_READ_CMD*>(cmd)->data.start_addr);
		DEBUG_PRINTF("Length = 0x%08X", static_cast<BL_MEM_READ_CMD*>(cmd)->data.length);
		break;
	case BL_VER_CMD_ID:
		DEBUG_PRINTLN(F("**** VER CMD ****"));
		printHeader(static_cast<BL_VER_CMD*>(cmd)->data.header);
		break;
	case BL_FLASH_ERASE_CMD_ID:
		DEBUG_PRINTLN(F("**** FLASH ERASE CMD ****"));
		printHeader(static_cast<BL_FLASH_ERASE_CMD*>(cmd)->data.header);
		DEBUG_PRINTF("Start address = 0x%08X", static_cast<BL_FLASH_ERASE_CMD*>(cmd)->data.address);
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
	cmd.reset();

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
	cmd.reset();

	uint8_t nack_field = 0xFF;
	bool ack_received = ReceiveAck(&nack_field);

	if (!ack_received)
		return false;

	ack_received = ReceiveAck(&nack_field);

	if (!ack_received)
		return false;

	return true;
}

bool Bootloader_Host::SendMemReadCommand(uint32_t start_address, uint32_t length, uint8_t out_buffer[]) {
	DEBUG_PRINTF("Reading from address 0x%08X, %ul bytes\n", start_address, length);
	BL_DATA_PACKET_CMD* data_block = nullptr;

	uint32_t total_bytes = 0;
	uint32_t retries = 0;
	uint32_t len = 0;
	bool more = true;

	std::unique_ptr<BL_MEM_READ_CMD> cmd = CreateMemReadCommand(start_address, length);
	if (!cmd.get())
		return false;
	printCommand(cmd.get(), BL_MEM_READ_CMD_ID);
	SendCommand(cmd.get()->serialized_data, sizeof(BL_MEM_READ_CMD));
	cmd.reset();

	/* Wait for ack on command */
	uint8_t nack_field = 0xFF;
	bool ack_received = ReceiveAck(&nack_field);

	if (!ack_received)
		return false;

	for (;;) {

		bool received = ReceivePacket(&len);

		if (!received)
			return false;

		data_block = (BL_DATA_PACKET_CMD*)rx_buffer;

		// Validate response
		uint32_t test_crc = bl_calculate_command_crc(data_block, data_block->data.header.payload_size);

		if (!VALIDATE_CMD(data_block->serialized_data, data_block->data.header.payload_size,
			data_block->data.header.CRC32))
		{
			DEBUG_PRINTF("Invalid CRC %08X", data_block->data.header.CRC32);
			DEBUG_PRINTF("Calculated CRC %08X", test_crc);
			SendAck(0, BL_NACK_INVALID_CRC);
			return false;
		}

		DEBUG_PRINTF("Received valid data packet, length = %d bytes",
			data_block->data.data_len);
		DEBUG_PRINTLN("First 20 bytes:");
		for (uint i = 0; i < 20; i++) {
			Serial.printf("0x%02X ", data_block->data.data_block[i]);
		}
		Serial.println();

		// TODO Copy data to out buffer
		size_t j = 0;
		for (size_t i = total_bytes; i < total_bytes + data_block->data.data_len; i++, j++)
		{
			out_buffer[i] = data_block->data.data_block[j];
		}

		total_bytes += data_block->data.data_len;

		/* Send ACK on last operation */
		SendAck(1, BL_NACK_SUCCESS);

		if (data_block->data.end_flag)
			break;
	}

	DEBUG_PRINTF("Total data received = %lu", total_bytes);
	return true;
}

bool Bootloader_Host::SendMemWriteCommand(uint32_t start_address, uint8_t data[], uint32_t data_size) {
	uint32_t number_of_blocks = data_size / BL_DATA_BLOCK_SIZE;
	uint32_t remainder_bytes = data_size % BL_DATA_BLOCK_SIZE;

	DEBUG_PRINTF("Number of blocks to send = %d", number_of_blocks);
	DEBUG_PRINTF("Remainder bytes = %d", remainder_bytes);

	std::unique_ptr<BL_MEM_WRITE_CMD> cmd = CreateMemWriteCommand(start_address);
	if (!cmd.get())
		return false;

	printCommand(cmd.get(), BL_MEM_WRITE_CMD_ID);
	SendCommand(cmd.get()->serialized_data, sizeof(BL_MEM_WRITE_CMD));
	cmd.reset();

	/* Wait for ack on command */
	uint8_t nack_field = 0xFF;
	bool ack_received = ReceiveAck(&nack_field);

	if (!ack_received)
		return false;

	/* To store the next block size to send */
	uint32_t next_block = BL_DATA_BLOCK_SIZE;
	bool flag = false;
	/* Proceed to send the rest of the data */
	for (size_t i = 0; i < number_of_blocks; i++)
	{
		/* If this is the last even block, assign next block size to remainder bytes size */
		if (i == number_of_blocks - 1)
			next_block = remainder_bytes;

		std::unique_ptr<BL_DATA_PACKET_CMD> block = CreateDataPacketCommand(&data[BL_DATA_BLOCK_SIZE * i], BL_DATA_BLOCK_SIZE, next_block,
			((i + 1) * BL_DATA_BLOCK_SIZE) == data_size);

		printCommand(block.get(), BL_DATA_PACKET_CMD_ID);

		if (!block.get())
			return false;
		yield();
		SendCommand(block.get()->serialized_data, block.get()->data.header.payload_size);

		/* Wait for ack on last packet*/
		uint8_t nack_field = 0xFF;
		bool ack_received = ReceiveAck(&nack_field);

		delay(1);
		if (!ack_received)
		{
			// Re-send
			i--;
			flag = true;
			continue;
		}

	}

	while (remainder_bytes)
	{
		std::unique_ptr<BL_DATA_PACKET_CMD> block = CreateDataPacketCommand(&data[BL_DATA_BLOCK_SIZE * number_of_blocks], remainder_bytes, 0,
			true);

		if (!block.get())
			return false;

		printCommand(block.get(), BL_DATA_PACKET_CMD_ID);
		SendCommand(block.get()->serialized_data, block.get()->data.header.payload_size);
		cmd.reset();

		/* Wait for ack on last packet*/
		uint8_t nack_field = 0xFF;
		bool ack_received = ReceiveAck(&nack_field);

		if (ack_received) {
			break;
		}
	}

	delay(10);
	return true;

}

bool Bootloader_Host::SendEnterCmdModeCommand() {
	std::unique_ptr<BL_ENTER_CMD_MODE_CMD> cmd = CreateEnterCmdModeCommand(ENTER_CMD_MODE_KEY);
	if (!cmd.get())
		return false;

	printCommand(cmd.get(), BL_ENTER_CMD_MODE_CMD_ID);
	SendCommand(cmd.get()->serialized_data, sizeof(BL_ENTER_CMD_MODE_CMD));
	cmd.reset();

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
	cmd.reset();

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

	/* Wait for any data to arrive */
	while (myPort.available() == 0);

	*length = myPort.available();
	myPort.readBytes(rx_buffer, sizeof(BL_CommandHeader_t));
	myPort.readBytes(&rx_buffer[sizeof(BL_CommandHeader_t)], ((BL_CommandHeader_t*)(rx_buffer))->payload_size);

	blinkLED(100);

	return true;
}

bool Bootloader_Host::ReceivePacket(uint32_t* length) {

	/* Wait for any data to arrive */
	while (myPort.available() == 0);
	*length = myPort.available();
	myPort.readBytes(rx_buffer, sizeof(BL_CommandHeader_t));
	myPort.readBytes(&rx_buffer[sizeof(BL_CommandHeader_t)], ((BL_CommandHeader_t*)(rx_buffer))->payload_size - sizeof(BL_CommandHeader_t));

	blinkLED(50);

	return true;
}


bool Bootloader_Host::ReceiveAck(uint8_t* nack_field) {

	/* Wait for any data to arrive */
	while (myPort.available() == 0);

	BL_ACK ack = { 0 };

	myPort.readBytes(ack.serialized_data, sizeof(BL_ACK));

	if (ack.data.field != 0xFF && nack_field)
		*nack_field = ack.data.field;

	printCommand(&ack, BL_ACK_CMD_ID);

	if (ack.data.ack)
		blinkLED(50);
	last_nack_fields = ack.data.field;
	return (ack.data.ack == 1);
}

bool Bootloader_Host::SendAck(uint8_t ack_value, BL_NACK_t field) {
	BL_ACK ack = { 0 };
	ack.data.cmd_id = BL_ACK_CMD_ID;
	ack.data.field = field;
	ack.data.ack = ack_value;
	myPort.write(ack.serialized_data, sizeof(BL_ACK));
	return true;
}
void Bootloader_Host::SyncClient() {
	uint8_t temp = 0;

	// Continuosly read from serial if received sync byte
	while (temp != SYNC_BYTE) {

		myPort.read(&temp, 1);
		// Exit loop so we don't send next sync byte
		if (temp == SYNC_BYTE)
			break;

		/* Send the sync byte then wait for a response */
		myPort.write((uint8_t*)&SYNC_BYTE, 1);
		delay(500);
	}
	delay(100);

	/* Synchronization successful */
	state = HostState::ReadyToSendCommand;

}



Bootloader_Host::~Bootloader_Host() {

}
