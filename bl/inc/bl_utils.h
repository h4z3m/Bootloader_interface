/**
 * @file bl_utils.h
 * @author Hazem Montasser (h4z3m.private@gmail.com)
 * @brief   Utility functions
 * @version 0.1
 * @date 2023-08-14
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef BL_UTILS_H_
#define BL_UTILS_H_
/*******************************************************************************
 *                              Includes                                       *
 *******************************************************************************/

#include <stdint.h>
#include <c_types.h>
/*******************************************************************************
 *                              Definitions                                    *
 *******************************************************************************/

#define VALIDATE_CMD(data, length, crc) \
    (bl_calculate_command_crc(data, length) == crc)

#define VALIDATE_ADDRESS_RANGE(start, end, max_length) \
    (start < end && (end - start + 1) <= max_length)

/*******************************************************************************
 *                            Public functions                                 *
 *******************************************************************************/

//uint32_t crc32(const uint8_t *data, uint32_t length);

#endif
