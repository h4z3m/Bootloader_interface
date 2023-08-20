#pragma once
#include "bl/inc/bl_cmd_types.h"
#include <memory>

#define CRC32_POLY 0xEDB88320

uint32_t crc32(const uint8_t* data, size_t length)
{
	uint32_t crc = 0xFFFFFFFF;

	for (size_t i = 0; i < length; i++)
	{
		crc ^= data[i];

		for (int j = 0; j < 8; j++)
		{
			crc = (crc >> 1) ^ ((crc & 1) * CRC32_POLY);
		}
	}

	return ~crc;
}

uint32_t bl_calculate_command_crc(void* command, uint32_t size)
{
	const uint8_t* data = (const uint8_t*)command;
	uint32_t crc = 0xFFFFFFFF;
	uint32_t crc_offset = offsetof(BL_CommandHeader_t, CRC32);

	// Calculate CRC for the command (excluding the CRC field)
	for (uint32_t i = 0; i < size; i++)
	{
		if ((i < crc_offset) || (i >= crc_offset + sizeof(uint32_t)))
		{
			crc ^= data[i];

			for (int j = 0; j < 8; j++)
			{
				crc = (crc >> 1) ^ ((crc & 1) * CRC32_POLY);
			}
		}
		/*if (size % 50)
			yield();*/
	}

	return ~crc;
}

class BootloaderCommand
{
protected:
	

public:
	BootloaderCommand() {}


	virtual ~BootloaderCommand() {}
};

class BL_VER_CMD_Builder : public BootloaderCommand
{
public:
	BL_VER_CMD_Builder()
	{
		cmd.data.header.payload_size = sizeof(BL_VER_CMD);
		cmd.data.header.cmd_id = BL_VER_CMD_ID;
	}

	BL_VER_CMD build()
	{
		cmd.data.header.CRC32 = bl_calculate_command_crc(&cmd, sizeof(BL_VER_CMD));
		return cmd;
	}

private:
	BL_VER_CMD cmd;
};

class BL_FLASH_ERASE_CMD_Builder : public BootloaderCommand
{
public:
	BL_FLASH_ERASE_CMD_Builder()
	{
		cmd.data.header.payload_size = sizeof(BL_FLASH_ERASE_CMD);
		cmd.data.header.cmd_id = BL_FLASH_ERASE_CMD_ID;
	}

	BL_FLASH_ERASE_CMD_Builder& setPageNumber(uint32_t page_number)
	{
		cmd.data.page_number = page_number; // Set the page_number field
		return *this;
	}

	BL_FLASH_ERASE_CMD_Builder& setPageCount(uint32_t page_count)
	{
		cmd.data.page_count = page_count; // Set the page_count field
		return *this;
	}

	BL_FLASH_ERASE_CMD build()
	{
		cmd.data.header.CRC32 = bl_calculate_command_crc(&cmd, sizeof(BL_FLASH_ERASE_CMD));
		return cmd;
	}

private:
	BL_FLASH_ERASE_CMD cmd;
};

class BL_MEM_WRITE_CMD_Builder : public BootloaderCommand
{
public:
	BL_MEM_WRITE_CMD_Builder()
	{
		cmd.data.header.payload_size = sizeof(BL_MEM_WRITE_CMD);
		cmd.data.header.cmd_id = BL_MEM_WRITE_CMD_ID;
	}

	BL_MEM_WRITE_CMD_Builder& setStartAddress(std::uint32_t startAddress)
	{
		cmd.data.start_address = startAddress; // Set the start_address field
		return *this;
	}

	BL_MEM_WRITE_CMD build()
	{
		cmd.data.header.CRC32 = bl_calculate_command_crc(&cmd, sizeof(BL_MEM_WRITE_CMD));
		return cmd;
	}

private:
	BL_MEM_WRITE_CMD cmd;
};

class BL_DATA_PACKET_CMD_Builder : public BootloaderCommand
{
public:
	BL_DATA_PACKET_CMD_Builder()
	{

		cmd.data.header.cmd_id = BL_DATA_PACKET_CMD_ID;
	}

	BL_DATA_PACKET_CMD_Builder& setEndFlag(bool flag)
	{
		cmd.data.end_flag = flag;
		return *this;
	}

	BL_DATA_PACKET_CMD_Builder& setData(uint8_t data[], uint32_t data_size)
	{
		memcpy(cmd.data.data_block, data, data_size);
		cmd.data.data_len = data_size;
		cmd.data.header.payload_size = sizeof(BL_DATA_PACKET_CMD) - 256 + data_size;
		return *this;
	}

	BL_DATA_PACKET_CMD_Builder& setNextBlockLen(uint32_t next_data_len)
	{
		/* If there's no next, set to zero*/
		if (next_data_len == 0)
			cmd.data.next_len = 0;
		else
			/* If there's a next block, calculate the whole packet size including data,
			subtract size of pointer because we will send the array it points to not the pointer */
			cmd.data.next_len = sizeof(BL_DATA_PACKET_CMD) - 256 + next_data_len;

		return *this;
	}

	BL_DATA_PACKET_CMD build()
	{
		cmd.data.header.CRC32 = bl_calculate_command_crc(&cmd, cmd.data.header.payload_size);
		return cmd;
	}

private:
	BL_DATA_PACKET_CMD cmd;
};

class BL_MEM_READ_CMD_Builder : public BootloaderCommand
{
public:
	BL_MEM_READ_CMD_Builder()
	{
		cmd.data.header.payload_size = sizeof(BL_MEM_WRITE_CMD);
		cmd.data.header.cmd_id = BL_MEM_READ_CMD_ID;
	}

	BL_MEM_READ_CMD_Builder& setStartAddress(std::uint32_t start_address)
	{
		cmd.data.start_addr = start_address; // Set the page_number field
		return *this;
	}

	BL_MEM_READ_CMD_Builder& setLength(std::uint32_t length)
	{
		cmd.data.length = length; // Set the length field
		return *this;
	}

	BL_MEM_READ_CMD build()
	{
		cmd.data.header.CRC32 = bl_calculate_command_crc(&cmd, sizeof(BL_MEM_READ_CMD));
		return cmd;
	}

private:
	BL_MEM_READ_CMD cmd;
};

class BL_GOTO_ADDR_CMD_Builder : public BootloaderCommand
{
public:
	BL_GOTO_ADDR_CMD_Builder()
	{
		cmd.data.header.payload_size = sizeof(cmd.data);
		cmd.data.header.cmd_id = BL_GOTO_ADDR_CMD_ID;
	}

	BL_GOTO_ADDR_CMD_Builder& setAddress(uint32_t address)
	{
		cmd.data.address = address; // Set the address field
		return *this;
	}

	BL_GOTO_ADDR_CMD build()
	{
		cmd.data.header.CRC32 = bl_calculate_command_crc(&cmd, sizeof(BL_GOTO_ADDR_CMD));
		return cmd;
	}

private:
	BL_GOTO_ADDR_CMD cmd;
};

class BL_ENTER_CMD_MODE_CMD_Builder : public BootloaderCommand
{
public:
	BL_ENTER_CMD_MODE_CMD_Builder()
	{
		cmd.data.header.payload_size = sizeof(BL_ENTER_CMD_MODE_CMD);
		cmd.data.header.cmd_id = BL_ENTER_CMD_MODE_CMD_ID;
	}

	BL_ENTER_CMD_MODE_CMD_Builder& setKey(uint32_t key)
	{
		cmd.data.key = key;
		return *this;
	}

	BL_ENTER_CMD_MODE_CMD build()
	{
		cmd.data.header.CRC32 = bl_calculate_command_crc(&cmd, sizeof(BL_ENTER_CMD_MODE_CMD));
		return cmd;
	}

private:
	BL_ENTER_CMD_MODE_CMD cmd;
};

class BL_JUMP_TO_APP_CMD_Builder : public BootloaderCommand
{
public:
	BL_JUMP_TO_APP_CMD_Builder()
	{
		cmd.data.header.payload_size = sizeof(BL_JUMP_TO_APP_CMD);
		cmd.data.header.cmd_id = BL_JUMP_TO_APP_CMD_ID;
	}

	BL_JUMP_TO_APP_CMD_Builder& setKey(uint32_t key)
	{
		cmd.data.key = key;
		return *this;
	}

	BL_JUMP_TO_APP_CMD build()
	{
		cmd.data.header.CRC32 = bl_calculate_command_crc(&cmd, sizeof(BL_JUMP_TO_APP_CMD));
		return cmd;
	}

private:
	BL_JUMP_TO_APP_CMD cmd;
};

std::unique_ptr<BL_GOTO_ADDR_CMD> CreateGotoAddrCommand(uint32_t address)
{
	BL_GOTO_ADDR_CMD_Builder builder;
	BL_GOTO_ADDR_CMD cmd = builder
		.setAddress(address)
		.build();
	return std::make_unique<BL_GOTO_ADDR_CMD>(cmd);
}

std::unique_ptr<BL_MEM_WRITE_CMD> CreateMemWriteCommand(uint32_t startAddress)
{
	BL_MEM_WRITE_CMD_Builder builder;
	BL_MEM_WRITE_CMD cmd = builder
		.setStartAddress(startAddress)
		.build();
	return std::make_unique<BL_MEM_WRITE_CMD>(cmd);
}

std::unique_ptr<BL_MEM_READ_CMD> CreateMemReadCommand(uint32_t startAddress, uint32_t length)
{
	BL_MEM_READ_CMD_Builder builder;
	BL_MEM_READ_CMD cmd = builder
		.setStartAddress(startAddress)
		.setLength(length)
		.build();
	return std::make_unique<BL_MEM_READ_CMD>(cmd);
}

std::unique_ptr<BL_VER_CMD> CreateVerCommand()
{
	BL_VER_CMD_Builder builder;
	BL_VER_CMD cmd = builder.build();
	return std::make_unique<BL_VER_CMD>(cmd);
}

std::unique_ptr<BL_FLASH_ERASE_CMD> CreateFlashEraseCommand(uint32_t startAddress, uint32_t page_count)
{
	BL_FLASH_ERASE_CMD_Builder builder;
	BL_FLASH_ERASE_CMD cmd = builder
		.setPageNumber(startAddress)
		.setPageCount(page_count)
		.build();
	return std::make_unique<BL_FLASH_ERASE_CMD>(cmd);
}

std::unique_ptr<BL_DATA_PACKET_CMD> CreateDataPacketCommand(uint8_t data[], uint32_t data_size, uint32_t next_block_len, bool end_flag)
{
	BL_DATA_PACKET_CMD_Builder builder;
	BL_DATA_PACKET_CMD cmd = builder
		.setData(data, data_size)
		.setEndFlag(end_flag)
		.setNextBlockLen(next_block_len)
		.build();

	return std::make_unique<BL_DATA_PACKET_CMD>(cmd);
}

std::unique_ptr<BL_JUMP_TO_APP_CMD> CreateJumpToAppCommand(uint32_t key)
{
	BL_JUMP_TO_APP_CMD_Builder builder;
	BL_JUMP_TO_APP_CMD cmd = builder
		.setKey(key)
		.build();
	return std::make_unique<BL_JUMP_TO_APP_CMD>(cmd);
}

std::unique_ptr<BL_ENTER_CMD_MODE_CMD> CreateEnterCmdModeCommand(uint32_t key)
{
	BL_ENTER_CMD_MODE_CMD_Builder builder;
	BL_ENTER_CMD_MODE_CMD cmd = builder
		.setKey(key)
		.build();
	return std::make_unique<BL_ENTER_CMD_MODE_CMD>(cmd);
}
