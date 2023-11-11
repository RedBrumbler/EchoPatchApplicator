#include "log.h"
#include "patches.hpp"


#include "echo-utils/shared/echo-utils.hpp"
#include "flamingo/shared/trampoline-allocator.hpp"
#include "flamingo/shared/trampoline.hpp"
#include "yodel/shared/modloader.h"

static constexpr auto ret_ins = 0xD65F03C0;
void hook_ovr_Message_GetUserArray(uint32_t* target) {
  if (!EchoUtils::protect(target, EchoUtils::RWX)) return;
  *target = ret_ins;
  EchoUtils::protect(target, EchoUtils::RX);
}

void hook_ovr_Message_GetDestinationArray(uint32_t* target) {
  if (!EchoUtils::protect(target, EchoUtils::RWX)) return;
  *target = ret_ins;
  EchoUtils::protect(target, EchoUtils::RX);
}

void hook_ovr_Message_GetPurchaseArray(uint32_t* target) {
  if (!EchoUtils::protect(target, EchoUtils::RWX)) return;
  *target = ret_ins;
  EchoUtils::protect(target, EchoUtils::RX);
}

void hook_ovr_Message_GetProductArray(uint32_t* target) {
  if (!EchoUtils::protect(target, EchoUtils::RWX)) return;
  *target = ret_ins;
  EchoUtils::protect(target, EchoUtils::RX);
}

void hook_ovr_Message_GetRoomInviteNotificationArray(uint32_t* target) {
  if (!EchoUtils::protect(target, EchoUtils::RWX)) return;
  *target = ret_ins;
  EchoUtils::protect(target, EchoUtils::RX);
}

#define HOOK(method)                                                                              \
  do {                                                                                            \
    auto lookup = EchoUtils::HandleUtils::get_symbol_address("libovrplatformloader.so", #method); \
    auto target = (uint32_t*)lookup.first;                                                        \
    if (!target) {                                                                                \
      LOG_ERROR("Could not hook " #method ": {}", lookup.second);                                 \
      break;                                                                                      \
    }                                                                                             \
    hook_##method(target);                                                                        \
  } while (0)

void install_ovr_hooks() {
  HOOK(ovr_Message_GetUserArray);
  HOOK(ovr_Message_GetDestinationArray);
  HOOK(ovr_Message_GetPurchaseArray);
  HOOK(ovr_Message_GetProductArray);
  HOOK(ovr_Message_GetRoomInviteNotificationArray);
}
