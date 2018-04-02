#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#define EMULATOR

#define SMP_RATE (20000)

#define multiply(a, b, n) (((a*b) + (1 << (n-1))) >> n)
#define divide(a, b, n) ((a << n)/b)

#endif // COMMON_H_INCLUDED
