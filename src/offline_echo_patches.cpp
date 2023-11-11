#include "log.h"

#include "echo-utils/shared/echo-utils.hpp"
#include "flamingo/shared/trampoline-allocator.hpp"
#include "flamingo/shared/trampoline.hpp"
#include "yodel/shared/modloader.h"

void patch_CR15Game_PreprocessCommandLine() {
  auto target = (uint32_t*)EchoUtils::AddressUtils::get_offset("libr15.so", 0x011f7ba8);
  if (!target) return;

  // make this b into the return instruction directly to act as if offline
  static constexpr auto ins = 0x1400002a;
  *target = ins;
}

void patch_CR15NetGame_LogInFailedCB() {
  auto target = (uint32_t*)EchoUtils::AddressUtils::get_offset("libr15.so", 0x0125f2cc);
  if (!target) return;

  // write a nop so we go into the LoginOffline if statement
  static constexpr auto nop_ins = 0xd503201f;
  *target = nop_ins;
}

void patch_CR15NetGame_LogInSuccess() {
  // FIXME: remove this return
  return;

  auto target_1 = (uint32_t*)EchoUtils::AddressUtils::get_offset("libr15.so", 0x0);
  auto target_2 = (uint32_t*)EchoUtils::AddressUtils::get_offset("libr15.so", 0x0);
  auto target_3 = (uint32_t*)EchoUtils::AddressUtils::get_offset("libr15.so", 0x0);
  // if one is 0, all are
  if (!target_1) return;

  // TODO: make more clear what these do, and actually implement
  // jne patch to alter flow logic
  *target_1 = 0;

  static constexpr auto nop_ins = 0xd503201f;
  // skip some stuff by patching out some checks
  *target_2 = nop_ins;
  *target_3 = nop_ins;
}

void patch_CR15NetGame_ServiceConnectionFailedCB() {
  auto target = (uint32_t*)EchoUtils::AddressUtils::get_offset("libr15.so", 0x0125ec28);
  if (!target) return;

  // Jump from the if to where RefreshServiceStatus is setup to be called
  static constexpr auto ins = 0x14000040;
  *target = ins;
}

void install_offline_patches() {
  patch_CR15Game_PreprocessCommandLine();
  patch_CR15NetGame_LogInFailedCB();
  patch_CR15NetGame_LogInSuccess();
  patch_CR15NetGame_ServiceConnectionFailedCB();
}
