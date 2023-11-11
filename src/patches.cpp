#include "patches.hpp"

#include "echo-utils/shared/echo-utils.hpp"
#include "log.h"

using namespace std::string_view_literals;

void install_entitlement_patch() {
  auto target = (uint32_t*)EchoUtils::AddressUtils::get_offset("libpnsovr.so", 0x20680c);
  if (!target) return;
  if (!EchoUtils::protect(target, EchoUtils::RWX)) return;

  *target = 0x14000007;
  EchoUtils::protect(target, EchoUtils::RX);
}

void install_prerequisites_pass_patch() {
  auto beq_1 = (uint32_t*)EchoUtils::AddressUtils::get_offset("libpnsovr.so", 0x1eda14);
  auto beq_2 = (uint32_t*)EchoUtils::AddressUtils::get_offset("libpnsovr.so", 0x1eda34);

  if (!beq_1) return;
  if (!EchoUtils::protect(beq_1, EchoUtils::RWX)) return;
  static constexpr auto nop_ins = 0xd503201f;

  *beq_1 = nop_ins;
  *beq_2 = nop_ins;
  EchoUtils::protect(beq_1, EchoUtils::RX);
}

void install_csysmodule_load_hook() {
  EchoUtils::Callbacks::add_library_callback(MOD_ID, "/pnsovr", [](auto handle) {
    CallbackPatch::install_loggedincb_hook();
    CallbackPatch::install_access_token_hook();
  });
}
