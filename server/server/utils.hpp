#pragma once
#include <stdint.h>

typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

enum
{
    IN_MILISECONDS = 1000,
    IN_MICROSECONDS = 1000,
    DIFF = 100,
    DISCONNECT_TIMEOUT = 30 * IN_MILISECONDS,
};