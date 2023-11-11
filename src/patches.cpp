#include "patches.hpp"

#include <asm-generic/mman-common.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <string>
#include <string_view>

#include "flamingo/shared/trampoline-allocator.hpp"
#include "flamingo/shared/trampoline.hpp"
#include "yodel/shared/modloader.h"

#include "log.h"
#include "util.hpp"


using namespace std::string_view_literals;

void* pns_ovr_handle;
uint32_t* pns_ovr_base;

void* get_pns_ovr_handle() {
  if (pns_ovr_handle) return pns_ovr_handle;

  pns_ovr_handle = dlopen("libpnsovr.so", RTLD_NOLOAD);
  if (!pns_ovr_handle) {
    LOG_ERROR("Could not get handle for libpnsovr.so: {}", dlerror());
  }
  return pns_ovr_handle;
}

uint32_t* get_pns_ovr_base() {
  if (pns_ovr_base) return pns_ovr_base;
  auto handle = get_pns_ovr_handle();
  if (!handle) return nullptr;

  pns_ovr_base = (uint32_t*)Util::baseAddr("libpnsovr.so");
  return pns_ovr_base;
}

uint32_t* get_entitlement_patch_address() {
  static constexpr auto offset = 0x20680c;
  auto base = get_pns_ovr_base();
  if (!base) return nullptr;

  return (uint32_t*)((uintptr_t)base + offset);
}

void install_entitlement_patch() {
  auto addr = get_entitlement_patch_address();
  if (!addr) return;
  if (!Util::protect(addr, PROT_READ | PROT_WRITE | PROT_EXEC)) return;

  *addr = 0x14000007;
  Util::protect(addr, PROT_READ | PROT_EXEC);
}

void install_prerequisites_pass_patch() {
  auto base = get_pns_ovr_base();
  if (!base) return;
  auto beq_1 = (uint32_t*)((uintptr_t)base + 0x1eda14);
  auto beq_2 = (uint32_t*)((uintptr_t)base + 0x1eda34);

  if (!Util::protect(beq_1, PROT_READ | PROT_WRITE | PROT_EXEC)) return;
  static constexpr auto nop_ins = 0xd503201f;

  *beq_1 = nop_ins;
  *beq_2 = nop_ins;
  Util::protect(beq_1, PROT_READ | PROT_EXEC);
}

uint32_t* get_csysmodule_load_address() {
  static auto constexpr label = "_ZN10NRadEngine10CSysModule4LoadERKNS_13CFixedStringTILy512EEEPvjj"sv;
  dlerror();
  auto address = static_cast<uint32_t*>(dlsym(modloader_r15_handle, label.data()));
  if (!address) {
    LOG_ERROR("Could not find CSysModule::Load address: {}", dlerror());
    return nullptr;
  }
  return address;
}

namespace CallbackPatch {
void install_loggedincb_hook();
void install_access_token_hook();
}  // namespace CallbackPatch

void install_csysmodule_load_hook() {
  auto target = get_csysmodule_load_address();
  if (!target) return;
  if (!Util::protect(target, PROT_READ | PROT_WRITE | PROT_EXEC)) return;

  static auto trampoline = flamingo::TrampolineAllocator::Allocate(128);
  trampoline.WriteHookFixups(target);
  trampoline.WriteCallback(&target[4]);
  trampoline.Finish();

  static auto csysmodule_load = [](char const* libname, void* param_2, uint param_3, uint param_4) noexcept -> void* {
    auto ret = reinterpret_cast<void* (*)(char const*, void*, uint, uint)>(trampoline.address.data())(libname, param_2,
                                                                                                      param_3, param_4);
    LOG_INFO("Library {} is being loaded", libname);
    if (std::string_view(libname) == "/pnsovr") {
      pns_ovr_handle = ret;
      // install_entitlement_patch();
      // install_prerequisites_pass_patch();

      CallbackPatch::install_loggedincb_hook();
      CallbackPatch::install_access_token_hook();
    }
    return ret;
  };

  size_t trampoline_size = 64;
  auto target_hook = flamingo::Trampoline(target, 8, trampoline_size);
  target_hook.WriteCallback(reinterpret_cast<uint32_t*>(+csysmodule_load));
  target_hook.Finish();

  Util::protect(target, PROT_READ | PROT_EXEC);
}
