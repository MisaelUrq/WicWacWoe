#if !defined(TYPES_H)
/* ========================================================================
   $File: File meant to hold the common and custom data types. $
   $Date: 2018-06-22 Jun $
   $Revision: $
   $Creator: Misael Urquieta $
   $Notice: $
   ======================================================================== */

#include <stdint.h>
#ifdef __cplusplus
#include <string>
#endif

#define global_var    static
#define internal_var  static
#define persist       static

#define Assert(__EXPRESSION) if(!(__EXPRESSION)) {*(volatile int *)0 = 0;}

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float    f32;
typedef double   f64;
typedef u32      b32;
typedef wchar_t  wchar;
typedef b32      bool32;
typedef size_t   usize;
typedef const char* str;

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define FOR_RANGE(start, end) for(u32 index = start; index < end; ++index)
#define INNER_FOR_RANGE(start, end) for(u32 inner_index = start; inner_index < end; ++inner_index)
#define FOR_RANGE_J(start, end) for(u32 j = start; j < end; ++j)

#define FOR_RANGE_I32(start, end) for(i32 index = start; index < end; ++index)


#define ArrayCount(array)     sizeof(array) / sizeof(array[0])

#define IsRange(__VAR__, __MIN__, __MAX__) (__VAR__ >= __MIN__ && __VAR__ <= __MAX__)

#ifdef __cplusplus
typedef std::string String;

#endif

#define TYPES_H
#endif
