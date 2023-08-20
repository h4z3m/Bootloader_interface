/**
 * @file DEBUG_UTILS.h
 * @author Hazem Montasser (h4z3m.private@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-07-26
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

/*******************************************************************************
 *                              Includes                                       *
 *******************************************************************************/

#include "UTILS.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

/*******************************************************************************
 *                              Definitions                                    *
 *******************************************************************************/

#define DEBUG_LEVEL_INFO (0)
#define DEBUG_LEVEL_WARN (1)
#define DEBUG_LEVEL_ERROR (2)
#define DEBUG_LEVEL_FATAL (3)
#define DEBUG_LEVEL_DEBUG (4)

#define DEBUG_LEVEL DEBUG_LEVEL_DEBUG

#ifndef TickCount
/**
 * @brief Returns the current tick count.
 * @note Configure according to the system timebase source.
 *
 */
#ifdef USE_HAL_DRIVER
#define TickCount() ((uint32_t)HAL_GetTick())
#else
#define TickCount() (0)
#endif
#endif

#ifdef DEBUG

#ifndef DEBUG_PRINTF

/**
 * @brief   Maximum buffer length for printf.
 *
 */
#define MAX_PRINTF_BUFFER_SIZE (256)

/**
 * @brief Prints a message to USB.
 *
 */
#define DEBUG_PRINTF(format, ...)                                        \
    do                                                                   \
    {                                                                    \
        uint8_t buffer[MAX_PRINTF_BUFFER_SIZE] = {0};                    \
        int length = snprintf(NULL, 0, format, ##__VA_ARGS__);           \
        snprintf((char *)buffer, sizeof(buffer), format, ##__VA_ARGS__); \
        CDC_Transmit_FS(buffer, length);                                 \
    } while (0)
#endif
/**
 * @brief Newline character for printf.
 * @note Configure according to the operating system.
 */
#define NEWLINE "\r\n"

/**
 * @brief Stringify a value.
 *
 */
#define STR(x) #x

/*******************************************************************************
 *                              Fatal log                                      *
 *******************************************************************************/

/**
 * @brief Prints a message with fatal information
 *
 */
#if DEBUG_LEVEL >= DEBUG_LEVEL_FATAL
#define DEBUG_FATAL(message, ...)  \
    DEBUG_PRINTF(                  \
        COLOR_RED                  \
        "%010lu"                   \
        ": " COLOR_RED             \
        "FATAL: " message NEWLINE, \
        TickCount(), ##__VA_ARGS__)
#else
#define DEBUG_FATAL(message, ...) \
    do                            \
    {                             \
    } while (0)
#endif

/*******************************************************************************
 *                              Error log                                      *
 *******************************************************************************/

#if DEBUG_LEVEL >= DEBUG_LEVEL_ERROR
/**
 * @brief Prints a message with error
 *
 */
#define DEBUG_ERROR(message, ...)              \
    DEBUG_PRINTF(                              \
        COLOR_BRIGHT_BLUE                      \
        "%010lu"                               \
        ": " COLOR_RED                         \
        "ERROR: " COLOR_RESET message NEWLINE, \
        TickCount(), ##__VA_ARGS__)
#else
#define DEBUG_ERROR(message, ...) \
    do                            \
    {                             \
    } while (0)
#endif

/*******************************************************************************
 *                              Warning log                                    *
 *******************************************************************************/

#if DEBUG_LEVEL >= DEBUG_LEVEL_WARN
/**
 * @brief Prints a message with warning
 *
 */
#define DEBUG_WARN(message, ...)                 \
    DEBUG_PRINTF(                                \
        COLOR_BRIGHT_BLUE                        \
        "%010lu"                                 \
        ": " COLOR_YELLOW                        \
        "WARNING: " COLOR_RESET message NEWLINE, \
        TickCount(), ##__VA_ARGS__)
#else
#define DEBUG_WARN(message, ...) \
    do                           \
    {                            \
    } while (0)
#endif

/*******************************************************************************
 *                              Information log                                *
 *******************************************************************************/

#if DEBUG_LEVEL >= DEBUG_LEVEL_INFO
/**
 * @brief Prints a message with information
 *
 */
#define DEBUG_INFO(message, ...)              \
    DEBUG_PRINTF(                             \
        COLOR_BRIGHT_BLUE                     \
        "%010lu"                              \
        ": " COLOR_GREEN                      \
        "INFO: " COLOR_RESET message NEWLINE, \
        TickCount(), ##__VA_ARGS__)
#else
#define DEBUG_INFO(message, ...) \
    do                           \
    {                            \
    } while (0)
#endif

#ifdef DEBUG
#define DEBUG_ASSERT(cond)                           \
    if (!(cond))                                     \
    {                                                \
        DEBUG_FATAL(__FILE__ ": Line #%lu: "         \
                             "Assertion failed: %s", \
                    __LINE__, #cond);                \
        for (;;)                                     \
            ;                                        \
    }
#else
#define DEBUG_ASSERT(cond) \
    do                     \
    {                      \
    } while (0)
#endif

#endif
#endif
