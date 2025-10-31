/********************************************************
 * Filename: hal/linux/type.h
 *
 * Author: Jiyong Park, RTOSLab. SNU
 * Modified by: Seongsoo Hong on 03/31/24
 *
 * Description: Definitions for data types
 ********************************************************/

#ifndef TYPE_H_
#define TYPE_H_

#include <stddef.h>
#include <stdint.h>

typedef uint8_t                 bool_t;
typedef uint8_t                 int8u_t;
typedef int8_t                  int8s_t;
typedef uint16_t                int16u_t;
typedef int16_t                 int16s_t;
typedef uint32_t                int32u_t;
typedef int32_t                 int32s_t;
typedef uint64_t                int64u_t;
typedef int64_t                 int64s_t;
typedef float                   fp32_t;
typedef double                  fp64_t;
typedef void                    *addr_t;

#endif
