#include <dlfcn.h>
#include <sys/mman.h>
#include "log.h"
#include "yodel/shared/loader.hpp"

// credits to https://github.com/ikoz/AndroidSubstrate_hookingC_examples/blob/master/nativeHook3/jni/nativeHook3.cy.cpp
uintptr_t baseAddr(char const* soname) {
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

bool protect(uint32_t* target, int protection) {
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

void install_skip_entitlement_hook() {
  auto pnsovr_handle = dlopen("libpnsovr.so", RTLD_GLOBAL | RTLD_NOW);
  if (!pnsovr_handle) {
    LOG_ERROR("Not installing skip entitlement patch, could not dlopen libpnsovr.so!");
    return;
  }

  auto base_addr = baseAddr("libpnsovr.so");
  if (!base_addr) {
    LOG_ERROR("Not installing skip entitlement patch, could not get the base address of libpnsovr.so!");
    return;
  }

  auto patch_addr = base_addr + 0x20680c;
  auto& ins = *((uint32_t*)patch_addr);
  LOG_INFO("Original instruction @ 0x{:X} (0x20680c): {:X}", patch_addr, ins);

  if (!protect((uint32_t*)patch_addr, PROT_READ | PROT_WRITE | PROT_EXEC)) return;

  LOG_DEBUG("Patching instruction to become: 0x14000007");
  ins = 0x14000007;
  LOG_DEBUG("Instruction is now: 0x{:X}", ins);

  protect((uint32_t*)patch_addr, PROT_READ | PROT_EXEC);
}

extern "C" void setup(CModInfo* info) {
  info->id = MOD_ID;
  info->version = VERSION;
  info->version_long = GIT_COMMIT;
}

extern "C" void load() {
  LOG_INFO("Patching entitlement check in echoVR from branch " GIT_BRANCH " (0x{:X}) by " GIT_USER, GIT_COMMIT);
  install_skip_entitlement_hook();
}
