#pragma once
#include <stdint.h>

struct Util {
  static uintptr_t baseAddr(char const* soname);
  static bool protect(uint32_t* target, int protection);
};
