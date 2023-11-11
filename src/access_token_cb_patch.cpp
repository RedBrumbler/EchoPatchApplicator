#include "Util.hpp"
#include "log.h"

#include <dlfcn.h>
#include <sys/mman.h>
#include "flamingo/shared/trampoline-allocator.hpp"
#include "flamingo/shared/trampoline.hpp"
#include "yodel/shared/modloader.h"

#include "log.h"

namespace CallbackPatch {

using namespace std::string_view_literals;

uint32_t* get_GotLoggedInUserAccessTokenCb_addr() {
  static constexpr auto symbol = "_ZN10NRadEngine10SCallbacks28GotLoggedInUserAccessTokenCbEP10ovrMessage"sv;
  auto addr = dlsym(dlopen("libpnsovr.so", RTLD_NOLOAD), symbol.data());
  if (!addr) {
    LOG_ERROR("Can't find symbol {} in libpnsovr.so!", symbol);
  }

  return (uint32_t*)addr;
}

void install_access_token_hook() {
  auto target = get_GotLoggedInUserAccessTokenCb_addr();
  if (!target) return;

  Util::protect(target, PROT_READ | PROT_WRITE | PROT_EXEC);
  static auto trampoline = flamingo::TrampolineAllocator::Allocate(128);
  trampoline.WriteHookFixups(target);
  trampoline.WriteCallback(&target[4]);
  trampoline.Finish();

  auto cb_hook = [](void* self, void* message) {
    LOG_INFO("Stopped errors being emitted from access token hook");
    // LOG_INFO("We passed {:x} as org scoped id, and {} as string id", get_orgScopedId(), get_sorgScopedId());
  };

  std::size_t ins_count = 8;
  std::size_t trampoline_size = 64;
  auto target_hook = flamingo::Trampoline(target, ins_count, trampoline_size);
  target_hook.WriteCallback(reinterpret_cast<uint32_t*>(+cb_hook));
  target_hook.Finish();

  Util::protect(target, PROT_READ | PROT_EXEC);
}

}  // namespace CallbackPatch
