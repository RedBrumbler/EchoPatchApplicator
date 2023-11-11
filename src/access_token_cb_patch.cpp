#include "log.h"

#include <dlfcn.h>
#include <sys/mman.h>
#include "echo-utils/shared/echo-utils.hpp"
#include "flamingo/shared/trampoline-allocator.hpp"
#include "flamingo/shared/trampoline.hpp"
#include "yodel/shared/modloader.h"

#include "log.h"

namespace CallbackPatch {

using namespace std::string_view_literals;

void install_access_token_hook() {
  auto lookup = EchoUtils::HandleUtils::get_symbol_address(
      "libpnsovr.so", "_ZN10NRadEngine10SCallbacks28GotLoggedInUserAccessTokenCbEP10ovrMessage");
  auto target = (uint32_t*)lookup.first;
  if (!target) return;

  if (!EchoUtils::protect(target, EchoUtils::RWX)) return;
  static auto trampoline = flamingo::TrampolineAllocator::Allocate(128);
  trampoline.WriteHookFixups(target);
  trampoline.WriteCallback(&target[4]);
  trampoline.Finish();

  static auto cb_hook = [](void* self, void* message) {
    LOG_INFO("Stopped errors being emitted from access token hook");
    // LOG_INFO("We passed {:x} as org scoped id, and {} as string id", get_orgScopedId(), get_sorgScopedId());
  };

  std::size_t ins_count = 8;
  std::size_t trampoline_size = 64;
  auto target_hook = flamingo::Trampoline(target, ins_count, trampoline_size);
  target_hook.WriteCallback(reinterpret_cast<uint32_t*>(+cb_hook));
  target_hook.Finish();

  EchoUtils::protect(target, EchoUtils::RX);
}

}  // namespace CallbackPatch
