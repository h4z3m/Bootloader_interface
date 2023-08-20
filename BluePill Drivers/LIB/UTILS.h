/**
 * @file UTILS.h
 * @author Hazem Montasser (h4z3m.private@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-07-26
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef UTILS_H
#define UTILS_H

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define AVG_INT(a, b) ((a) + (b)) / 2
#define AVG_FLOAT(a, b) ((((float)a) + (float)(b)) / 2.0f)

#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_BLUE "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN "\x1b[36m"
#define COLOR_WHITE " \033[0;97m"
#define COLOR_RESET "\x1b[0m"

#define COLOR_BRIGHT_BLACK "\x1b[90m"
#define COLOR_BRIGHT_RED "\x1b[91m"
#define COLOR_BRIGHT_GREEN "\x1b[92m"
#define COLOR_BRIGHT_YELLOW "\x1b[93m"
#define COLOR_BRIGHT_BLUE "\x1b[94m"
#define COLOR_BRIGHT_MAGENTA "\x1b[95m"
#define COLOR_BRIGHT_CYAN "\x1b[96m"
#define COLOR_BRIGHT_WHITE "\x1b[97m"

#endif
