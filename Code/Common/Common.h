#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

//Required for memset.
#include <string.h>

//Replication of patchblocks emulation define.
#define EMULATOR

//Replication of patchblocks constant.
#define SMP_RATE (20000)

//Handy reminder of how to multiply and divide with fixed point.
//#define multiply(a, b, n) ((((a)*(b)) + (1 << ((n)-1))) >> (n))
//#define divide(a, b, n) (((a) << (n))/(b))

//Not using this but might in the future.
//#define dbg(_fmt, ...)

#endif // COMMON_H_INCLUDED
