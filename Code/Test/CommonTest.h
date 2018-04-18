#ifndef COMMON_TEST_H_INCLUDED
#define COMMON_TEST_H_INCLUDED

#include <stdio.h>      //printf
#include <stdlib.h>     //qsort
#include <math.h>       //roundf
#include <stdint.h>     //uint32_t etc

#define TEST_ASSERT(condition)  if(0 == (condition)) {printf("failed \n"); __builtin_trap();}

#define toFX(a, n) (roundf(a *(1 << n)))

#define toFloat(a, n) ((float)(a) / (1 << n))

#endif // COMMON_H_INCLUDED
