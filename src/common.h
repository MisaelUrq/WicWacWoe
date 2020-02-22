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

#define strlenwnull(str) strlen(str)+1

struct v2u {
    u32 x;
    u32 y;
};

struct v2f {
    f32 x;
    f32 y;
};

#define IsRange(__VAR__, __MIN__, __MAX__) (__VAR__ >= __MIN__ && __VAR__ <= __MAX__)

#ifdef __cplusplus
typedef std::string String;

/* NOTE: I think some compilers have this function name already taken
 * in stdio.h, and it does the exact same thing. So it's only here for
 * the MSVC compiler, where I could't finded. But it has its
 * parameters in a diferent order so in a cpp compiler it would not
 * collide */
static inline char *fgetdelim(FILE *file, char* buf, u32 count, const char delim)
{
    u32 i = 0;
    while ( i < count-1 && fgets(&buf[i], 2, file)) {
        if (feof(file)) {
            buf[i] = 0;
            return 0;
        } else if (buf[i] == delim) {
            i = 0;
            return &buf[i];
        }
        i += 1;
    }
    return 0;
}

#endif

#define TYPES_H
#endif
