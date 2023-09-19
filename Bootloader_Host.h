#pragma once
#include "bl_cmd_types.h"
#include <SoftwareSerial.h>
#include <stdint.h>
#include <iostream>

#define LED (D0)

#define MYPORT_TX 12
#define MYPORT_RX 14

class Bootloader_Host
{
	static Bootloader_Host* instance; // Singleton instance pointer

	enum class HostState
	{
		Synchronization,
		SendingData,
		ReadyToSendCommand,
		WaitingForAck
	};

	const int SYNC_BYTE = 0xA5;						// Magic byte to synchronize
	const uint32_t JUMP_APP_KEY = 0x4032AFE5;		// Magic key to jump to app
	const uint32_t ENTER_CMD_MODE_KEY = 0x09B21FFC; // Magic key to enter cmd mode

	uint8_t rx_buffer[1512];						  // Receive buffer
	SoftwareSerial myPort;						  // Software serial interface
	HostState state = HostState::Synchronization; // Current state
	Bootloader_Host();

public:
	BL_NACK_t last_nack_fields;
	/**
	 * @brief Get the Instance object
	 *
	 * @return * Bootloader_Host*
	 */
	static Bootloader_Host* getInstance();

	/**
	 * @brief Sends version command
	 *
	 * @return uint8_t Version value of the client
	 * @return uint8_t 0 If the command failed
	 */
	uint8_t SendVersionCommand();

	/**
	 * @brief Sends a flash erase command which erases contiguous pages
	 *
	 * @param page_start_address The address of the page at which to start erasing
	 * @param page_count 		 The number of pages to erase starting at the start address
	 * @return true		If operation was success
	 * @return false 	If operation was failure (due to error in inputs or other)
	 */
	bool SendFlashEraseCommand(uint32_t page_start_address, uint32_t page_count);

	/**
	 * @brief Sends a memmory read command to the client which reads from the start address
	 *
	 * @param start_address The start address at which to write data
	 * @param length		The length of the data to read in bytes
	 * @param out_buffer	Out buffer to read data into. Must be of appropriate length.
	 * @return true 		If operation was success
	 * @return false 		If operation was failure (due to error in inputs or other)
	 */
	bool SendMemReadCommand(uint32_t start_address, uint32_t length, uint8_t out_buffer[]);

	/**
	 * @brief Sends a memmory write command to the client which writes at the start address
	 *
	 * @param start_address The start address at which to write data
	 * @param data 			The whole data array to write
	 * @param data_size 	The size of the data in bytes
	 * @return true 		If operation was success
	 * @return false 		If operation was failure (due to error in inputs or other)
	 */
	bool SendMemWriteCommand(uint32_t start_address, uint8_t data[], uint32_t data_size);

	/**
	 * @brief	Sends enter command mode command which allows the client to send commands
	 *
	 * @return true
	 * @return false
	 */
	bool SendEnterCmdModeCommand();

	/**
	 * @brief 	Sends a jump to app command
	 *
	 * @return true
	 * @return false
	 */
	bool SendJumpToAppCommand();

	// Override copy constructor to prevent copying
	Bootloader_Host(const Bootloader_Host& obj) = delete;
	~Bootloader_Host();

private:
	/**
	 * @brief	Prints the header of the command
	 *
	 * @param header
	 */
	void printHeader(BL_CommandHeader_t& header);

	/**
	 * @brief	Prints the command
	 *
	 * @param cmd	Pointer to the command structure
	 * @param id 	ID of the command
	 */
	void printCommand(void* cmd, BL_CommandID_t id);

	/**
	 * @brief 	Blinks the LED
	 *
	 * @param duration Duration in milliseconds
	 */
	void blinkLED(int duration = 1000);

	/**
	 * @brief 	Receives a response of specified length
	 *
	 * @param length
	 * @return true 	If a response was received
	 * @return false 	If a response was not received or an error occurred
	 */
	bool ReceiveResponse(uint32_t* length);

	bool ReceivePacket(uint32_t* length);
	/**
	 * @brief 	Receives an ack
	 *
	 * @param nack_field
	 * @return true 	If an ack was received
	 * @return false 	If an ack was not received or an error occurred
	 */
	bool ReceiveAck(uint8_t* nack_field);

	/**
	 * @brief 	Sends an ack
	 *
	 * @return true
	 * @return false
	 */
	bool SendAck(uint8_t ack_value, BL_NACK_t field);

	/**
	 * @brief 	Synchronizes the host with the client
	 * @note 	This function blocks until the host is synchronized
	 */
	void SyncClient(void);

	/**
	 * @brief 	Sends a command to the client
	 *
	 * @param data	The serialized data to send
	 * @param bytes The number of bytes to send
	 */
	void SendCommand(uint8_t* data, uint32_t bytes);
};
