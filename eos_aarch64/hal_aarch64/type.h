/********************************************************
 * Filename: hal/linux/type.h
 * 
 * Author: wsyoo, RTOSLab. SNU.
 * 
 * Description: data type definitions
 ********************************************************/
#ifndef TYPE_H_
#define TYPE_H_
typedef unsigned char	    bool_t;
typedef unsigned char	    int8u_t;
typedef signed char		    int8s_t;
typedef unsigned short	    int16u_t;
typedef signed short	    int16s_t;
typedef unsigned int	    int32u_t;
typedef signed int		    int32s_t;
typedef unsigned long long  int64u_t;   // unsigned long int
typedef float			    fp32_t;
typedef double			    fp64_t;
typedef void			    *addr_t;
typedef unsigned long	    size_t;     // 64bit 변경

#endif // TYPE_H_
