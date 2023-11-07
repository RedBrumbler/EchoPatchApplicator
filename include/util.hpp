#pragma once
#include <stdint.h>

extern uintptr_t baseAddr(char const* soname);
extern bool protect(uint32_t* target, int protection);
