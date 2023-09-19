/**
 * @file bl_cmd_types.h
 * @author Hazem Montasser (h4z3m.private@gmail.com)
 * @brief	Bootloader command types
 * @version 0.1
 * @date 2023-08-14
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef BL_CMD_TYPES_H_
#define BL_CMD_TYPES_H_

 /*******************************************************************************
  *                              Includes                                       *
  *******************************************************************************/

#include <stdint.h>

#define BL_PACKED_ALIGNED __attribute__((packed, aligned(1)))

#define BL_DATA_BLOCK_SIZE (1024U)
  /*******************************************************************************
   *							Typedefs						        		   *
   *******************************************************************************/

   /**
	* @enum	BL_CommandID
	* @brief	Available bootloader commands
	*
	*/
typedef enum
__attribute__((packed))
{
	BL_GOTO_ADDR_CMD_ID = 0x01, /**< BL_GOTO_ADDR_CMD_ID */
		BL_MEM_WRITE_CMD_ID,		/**< BL_MEM_WRITE_CMD_ID */
		BL_MEM_READ_CMD_ID,			/**< BL_MEM_READ_CMD_ID */
		BL_VER_CMD_ID,				/**< BL_VER_CMD_ID */
		BL_FLASH_ERASE_CMD_ID,		/**< BL_FLASH_ERASE_CMD_ID */
		BL_ACK_CMD_ID,				/**< BL_ACK_CMD_ID */
		BL_ENTER_CMD_MODE_CMD_ID,	/**< BL_ENTER_CMD_MODE_CMD_ID */
		BL_JUMP_TO_APP_CMD_ID,		/**< BL_JUMP_TO_APP_CMD_ID */
		BL_DATA_PACKET_CMD_ID,		/**< BL_DATA_PACKET_CMD_ID */
		BL_RESPONSE_CMD_ID = 0xFF	/**< BL_RESPONSE_CMD_ID */
} BL_CommandID_t;

typedef enum
__attribute__((packed))
{
	BL_NACK_SUCCESS = 0,
		BL_NACK_INVALID_CMD = 1 << 0,
		BL_NACK_INVALID_KEY = 1 << 1,
		BL_NACK_INVALID_ADDRESS = 1 << 2,
		BL_NACK_INVALID_LENGTH = 1 << 3,
		BL_NACK_INVALID_DATA = 1 << 4,
		BL_NACK_INVALID_CRC = 1 << 5,
		BL_NACK_OPERATION_FAILURE = 1 << 6
} BL_NACK_t;

/* Received commands */

/**
 * @struct
 * @brief	Bootloader command header. Must be at the top of any command
 *
 */
typedef struct BL_PACKED_ALIGNED
{
	uint32_t payload_size;
	BL_CommandID_t cmd_id;
	uint32_t CRC32;
} BL_CommandHeader_t;

/**
 * @union
 * @brief	Union representing the received "ENTER CMD MODE" command.
 *
 */
typedef union BL_PACKED_ALIGNED
{
	uint8_t serialized_data[sizeof(BL_CommandHeader_t) + 4];
	struct BL_PACKED_ALIGNED
	{
		BL_CommandHeader_t header;
		uint32_t key;
	} data;
} BL_ENTER_CMD_MODE_CMD;

/**
 * @union BL_GOTO_ADDR_CMD
 * @brief Union representing the received "GO TO ADDR" command.
 *
 */
typedef union BL_PACKED_ALIGNED
{
	uint8_t serialized_data[sizeof(BL_CommandHeader_t) + 4];
	struct BL_PACKED_ALIGNED
	{
		BL_CommandHeader_t header;
		uint32_t address; /**< Address */
	} data;
} BL_GOTO_ADDR_CMD;

/**
 * @union BL_MEM_WRITE_CMD
 * @brief Union representing the received "MEM WRITE" command.
 *
 */
typedef union BL_PACKED_ALIGNED
{
	uint8_t serialized_data[sizeof(BL_CommandHeader_t) + 4];
	struct BL_PACKED_ALIGNED
	{
		BL_CommandHeader_t header;
		uint32_t start_address;
	} data;
} BL_MEM_WRITE_CMD;

/**
 * @union BL_MEM_READ_CMD
 * @brief Union representing the received "MEM READ" command.
 *
 */
typedef union BL_PACKED_ALIGNED
{
	uint8_t serialized_data[sizeof(BL_CommandHeader_t) + 8];
	struct BL_PACKED_ALIGNED
	{
		BL_CommandHeader_t header;
		uint32_t start_addr; /**< Start address */
		uint32_t length;	 /**< Length of data to read */
	} data;
} BL_MEM_READ_CMD;

/**
 * @union BL_FLASH_ERASE_CMD
 * @brief Union representing the received "FLASH ERASE" command.
 *
 */
typedef union BL_PACKED_ALIGNED
{
	uint8_t serialized_data[sizeof(BL_CommandHeader_t) + 8];
	struct BL_PACKED_ALIGNED
	{
		BL_CommandHeader_t header;
		uint32_t address;	 /**< Page address */
		uint32_t page_count; /**< Page count */
	} data;
} BL_FLASH_ERASE_CMD;

/**
 * @union BL_VER_CMD
 * @brief Union representing the received "VERSION" command.
 *
 */
typedef union BL_PACKED_ALIGNED
{
	uint8_t serialized_data[sizeof(BL_CommandHeader_t)];
	struct BL_PACKED_ALIGNED
	{
		BL_CommandHeader_t header;
	} data;
} BL_VER_CMD;

/**
 * @union	BL_DATA_PACKET_CMD
 * @brief	Union representing the received "DATA PACKET" command.
 *
 */
typedef union BL_PACKED_ALIGNED
{
	uint8_t serialized_data[sizeof(BL_CommandHeader_t) + BL_DATA_BLOCK_SIZE + 9];
	struct BL_PACKED_ALIGNED
	{
		BL_CommandHeader_t header;
		uint32_t data_len;
		uint32_t next_len;
		uint8_t end_flag;
		uint8_t data_block[BL_DATA_BLOCK_SIZE];
	} data;
} BL_DATA_PACKET_CMD;

/**
 * @union	BL_JUMP_TO_APP_CMD
 * @brief	Union representing the received "JUMP TO APP" command.
 *
 */
typedef union BL_PACKED_ALIGNED
{
	uint8_t serialized_data[sizeof(BL_CommandHeader_t) + 4];
	struct BL_PACKED_ALIGNED
	{
		BL_CommandHeader_t header;
		uint32_t key; /**< Magic key value */
	} data;
} BL_JUMP_TO_APP_CMD;

/* Sent data */

/**
 * @union BL_ACK
 * @brief Union representing the sent ACK command.
 *
 */
typedef union BL_PACKED_ALIGNED
{
	uint8_t serialized_data[3];
	struct BL_PACKED_ALIGNED
	{
		BL_CommandID_t cmd_id; /**< Command ID */
		uint8_t ack;		   /**< ACK value */
		BL_NACK_t field;	   /**< NACK field */
	} data;
} BL_ACK;

/**
 * @union BL_Response
 * @brief Union representing the response data.
 *
 */
typedef union BL_PACKED_ALIGNED
{
	uint8_t serialized_data[sizeof(BL_CommandHeader_t) + 8];
	struct BL_PACKED_ALIGNED
	{
		BL_CommandHeader_t header;
		uint8_t data[8]; /**< Data */
	} data;
} BL_Response;

/**
 * @struct BL_Response_data
 * @brief Structure representing the response data with crc.
 *
 */
typedef struct BL_PACKED_ALIGNED
{
	uint8_t data[8]; /**< Data */
	uint32_t crc;	 /**< CRC */
} BL_Response_data;

#endif /* BL_CMD_TYPES_H_ */
