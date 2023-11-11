#include "log.h"

#include <dlfcn.h>
#include <inttypes.h>
#include <sys/mman.h>
#include <cstdio>

#include "echo-utils/shared/echo-utils.hpp"
#include "flamingo/shared/trampoline-allocator.hpp"
#include "flamingo/shared/trampoline.hpp"
#include "yodel/shared/modloader.h"

namespace CallbackPatch {

using namespace std::string_view_literals;

uint32_t* get_GotLoggedInUserOrgIdCb_addr() {
  static constexpr auto symbol = "_ZN10NRadEngine10SCallbacks22GotLoggedInUserOrgIdCbEP10ovrMessage"sv;
  auto addr = dlsym(dlopen("libpnsovr.so", RTLD_NOLOAD), symbol.data());
  if (!addr) {
    LOG_ERROR("Can't find symbol {} in libpnsovr.so!", symbol);
  }

  return (uint32_t*)addr;
}

uint64_t* gOrgScopedId;
uint64_t& get_orgScopedId() {
  if (gOrgScopedId) return *gOrgScopedId;

  gOrgScopedId = (uint64_t*)EchoUtils::AddressUtils::get_offset("libpnsovr.so", 0x70e3e0);
  return *gOrgScopedId;
}

char* sOrgScopedId;
char* get_sorgScopedId() {
  if (sOrgScopedId) return sOrgScopedId;

  sOrgScopedId = (char*)EchoUtils::AddressUtils::get_offset("libpnsovr.so", 0x70e458);
  return sOrgScopedId;
}

extern void install_loggedincb_hook() {
  auto lookup = EchoUtils::HandleUtils::get_symbol_address(
      "libpnsovr.so", "_ZN10NRadEngine10SCallbacks22GotLoggedInUserOrgIdCbEP10ovrMessage");
  auto target = (uint32_t*)lookup.first;
  if (!target) {
    LOG_ERROR("Could not find symbol _ZN10NRadEngine10SCallbacks22GotLoggedInUserOrgIdCbEP10ovrMessage: {}",
              lookup.second);
    return;
  }

  EchoUtils::protect(target, EchoUtils::RWX);
  static auto trampoline = flamingo::TrampolineAllocator::Allocate(128);
  trampoline.WriteHookFixups(target);
  trampoline.WriteCallback(&target[4]);
  trampoline.Finish();

  static auto cb_hook = [](void* self, void* message) {
    // do something to make the app think we succeeded!
    get_orgScopedId() = 0xDEADBEEFDEADBEEF;
    std::snprintf(get_sorgScopedId(), 20, "%lu", 0xDEADBEEFDEADBEEF);
    LOG_INFO("We passed 0x{:x} as org scoped id, and {} as string id", get_orgScopedId(), get_sorgScopedId());
  };

  std::size_t ins_count = 8;
  std::size_t trampoline_size = 64;
  auto target_hook = flamingo::Trampoline(target, ins_count, trampoline_size);
  target_hook.WriteCallback(reinterpret_cast<uint32_t*>(+cb_hook));
  target_hook.Finish();

  EchoUtils::protect(target, EchoUtils::RX);
}
}  // namespace CallbackPatch
