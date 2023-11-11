#include "log.h"
#include "util.hpp"


#include <dlfcn.h>
#include <sys/mman.h>
#include <cstdio>

#include "flamingo/shared/trampoline-allocator.hpp"
#include "flamingo/shared/trampoline.hpp"
#include "yodel/shared/modloader.h"

uint32_t* r15_base;
uint32_t* get_r15_base() {
  if (r15_base) return r15_base;
  r15_base = (uint32_t*)Util::baseAddr("libr15-original.so");
  if (!r15_base) {
    LOG_ERROR("libr15-original.so base address not found!");
  }
  return r15_base;
}

void patch_CR15Game_PreprocessCommandLine() {
  auto base = get_r15_base();
  if (!base) return;
  static constexpr auto offset = 0x011f7ba8;
  auto addr = (uint32_t*)((uintptr_t)base + offset);

  // make this b into the return instruction directly to act as if offline
  static constexpr auto ins = 0x1400002a;
  *addr = ins;
}

void patch_CR15NetGame_LogInFailedCB() {
  auto base = get_r15_base();
  if (!base) return;
  static constexpr auto offset = 0x0125f2cc;
  auto addr = (uint32_t*)((uintptr_t)base + offset);

  // write a nop so we go into the LoginOffline if statement
  static constexpr auto nop_ins = 0xd503201f;
  *addr = nop_ins;
}

void patch_CR15NetGame_LogInSuccess() {
  // FIXME: remove this return
  return;

  auto base = get_r15_base();
  if (!base) return;

  auto patch_1 = (uint32_t*)((uintptr_t)base + 0x0);
  auto patch_2 = (uint32_t*)((uintptr_t)base + 0x0);
  auto patch_3 = (uint32_t*)((uintptr_t)base + 0x0);

  // TODO: make more clear what these do, and actually implement
  // jne patch to alter flow logic
  *patch_1 = 0;

  static constexpr auto nop_ins = 0xd503201f;
  // skip some stuff by patching out some checks
  *patch_2 = nop_ins;
  *patch_3 = nop_ins;
}

void patch_CR15NetGame_ServiceConnectionFailedCB() {
  auto base = get_r15_base();
  if (!base) return;
  static constexpr auto offset = 0x0125ec28;
  auto addr = (uint32_t*)((uintptr_t)base + offset);

  // Jump from the if to where RefreshServiceStatus is setup to be called
  static constexpr auto ins = 0x14000040;
  *addr = ins;
}

void install_offline_patches() {
  patch_CR15Game_PreprocessCommandLine();
  patch_CR15NetGame_LogInFailedCB();
  patch_CR15NetGame_LogInSuccess();
  patch_CR15NetGame_ServiceConnectionFailedCB();
}
