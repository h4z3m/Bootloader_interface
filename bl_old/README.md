# CURT Bootloader for STM32F103

## Communication procedures

Specific communication protocol is abstracted from the bootloader. Any appropriate protocol can be used to communicate with the bootloader. The only thing that is required is overloading the weak functions 'BL_send' and 'BL_receive' that are used internally for the command handling.

Currently, CURT_BL supports supports these commands:
    - BL_GOTO_ADDR_CMD
    - BL_MEM_WRITE_CMD
      - Writes the received data to flash memory
    - BL_MEM_READ_CMD
      - Reads data from flash memory
    - BL_VER_CMD
      - Sends version information
    - BL_FLASH_ERASE_CMD
      - Erases flash memory
    - BL_ACK_CMD
      - Acknowledge command
    - BL_RESPONSE_CMD
      - Response command

### BL_VER_CMD Procedure

1. Client sends BL_VER_CMD
2. BL sends BL_ACK_CMD
   1. If failed, BL sends BL_ACK_CMD with negative ack with the errored field.
3. BL ends BL_RESPONSE_CMD with the version info at data[0]

### BL_MEM_WRITE_CMD Procedure

1. Client sends BL_MEM_WRITE_CMD with the first block size included in 'next_block_len' including CRC32.
2. BL sends BL_ACK_CMD.
   1. If failed, BL sends BL_ACK_CMD with negative ack with the errored field.
3. When client receives positive ACK, it must send data blocks to BL:
   1. For every block successfully received, the BL writes it to memory, then sends a positive ACK.
   2. If the block is corrupted, a negative ack is sent, with the errored field set and the procedure is aborted.
   3. For the last block, the client must set the 'end_flag' field to '1' to indicate the end of the memory read.
