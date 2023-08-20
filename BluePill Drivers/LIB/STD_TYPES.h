/*******************************************************************************
 *                                                                                *
 *     Module          : Standard Types                                            *
 *                                                                                *
 *     File Name       : STD_TYPES.h                                               *
 *                                                                                *
 *     Author          : AbdElRahman Sabry                                         *
 *                                                                                *
 *    Date            : 23/9/2021                                                 *
 *                                                                                *
 *    Version         : v1                                                        *
 *                                                                                 *
 *******************************************************************************/

#ifndef STDTYPES_H_
#define STDTYPES_H_

#ifdef __cplusplus
extern "C"
{
#endif

    // Standard Data Types
    typedef unsigned char u8;
    typedef signed char s8;
    typedef unsigned short u16;
    typedef signed short s16;
    typedef unsigned int u32;
    typedef signed int s32;
    typedef unsigned long long u64;
    typedef signed long long s64;
    typedef float f32;
    typedef double f64;

    // Booleans
#ifndef __cplusplus
    typedef unsigned char bool;
#endif

#ifndef TRUE
#define TRUE (1u)
#endif

#ifndef FALSE
#define FALSE (0u)
#endif

#ifndef HIGH
#define HIGH (1u)
#endif

#ifndef LOW
#define LOW (0u)
#endif

#ifndef ONE
#define ONE (1u)
#endif

#ifndef ZERO
#define ZERO (0u)
#endif

    // Pointers
#ifndef NULLPTR
#define NULLPTR ((void *)0)
#endif

#ifndef DISABLE
#define DISABLE (0U)
#endif

#ifndef ENABLE
#define ENABLE (1U)
#endif

#ifdef __cplusplus
}
#endif

#endif /* STDTYPES_H_ */
