#include "Util.hpp"
#include <sys/mman.h>
#include "log.h"

// credits to https://github.com/ikoz/AndroidSubstrate_hookingC_examples/blob/master/nativeHook3/jni/nativeHook3.cy.cpp
uintptr_t Util::baseAddr(char const* soname) {
  if (soname == NULL) return (uintptr_t)NULL;

  FILE* f = NULL;
  char line[200] = { 0 };
  char* state = NULL;
  char* tok = NULL;
  char* baseAddr = NULL;
  if ((f = fopen("/proc/self/maps", "r")) == NULL) return (uintptr_t)NULL;
  while (fgets(line, 199, f) != NULL) {
    tok = strtok_r(line, "-", &state);
    baseAddr = tok;
    strtok_r(NULL, "\t ", &state);
    strtok_r(NULL, "\t ", &state);        // "r-xp" field
    strtok_r(NULL, "\t ", &state);        // "0000000" field
    strtok_r(NULL, "\t ", &state);        // "01:02" field
    strtok_r(NULL, "\t ", &state);        // "133224" field
    tok = strtok_r(NULL, "\t ", &state);  // path field

    if (tok != NULL) {
      int i;
      for (i = (int)strlen(tok) - 1; i >= 0; --i) {
        if (!(tok[i] == ' ' || tok[i] == '\r' || tok[i] == '\n' || tok[i] == '\t')) break;
        tok[i] = 0;
      }
      {
        size_t toklen = strlen(tok);
        size_t solen = strlen(soname);
        if (toklen > 0) {
          if (toklen >= solen && strcmp(tok + (toklen - solen), soname) == 0) {
            fclose(f);
            return (uintptr_t)strtoll(baseAddr, NULL, 16);
          }
        }
      }
    }
  }
  fclose(f);
  return (uintptr_t)NULL;
}

bool Util::protect(uint32_t* target, int protection) {
  constexpr static auto kPageSize = 4096ULL;

  auto* page_aligned_target = reinterpret_cast<uint32_t*>(reinterpret_cast<uint64_t>(target) & ~(kPageSize - 1));
  LOG_DEBUG("Marking target: {} as writable, page aligned: {}", fmt::ptr(target), fmt::ptr(page_aligned_target));
  if (::mprotect(page_aligned_target, kPageSize, protection) != 0) {
    // Log error on mprotect!
    LOG_ERROR("Failed to mark: {} (page aligned: {}). err: {}", fmt::ptr(target), fmt::ptr(page_aligned_target),
              std::strerror(errno));
    return false;
  }
  return true;
}
