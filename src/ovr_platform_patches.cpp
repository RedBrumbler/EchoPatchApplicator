#include "Util.hpp"
#include "log.h"

#include <dlfcn.h>
#include <sys/mman.h>
#include <cstdio>

#include "flamingo/shared/trampoline-allocator.hpp"
#include "flamingo/shared/trampoline.hpp"
#include "yodel/shared/modloader.h"

void* ovrplatformloader_handle;
uint32_t* ovrplatformloader_base;

void* get_ovrplatformloader_handle() {
  if (ovrplatformloader_handle) return ovrplatformloader_handle;
  ovrplatformloader_handle = dlopen("libovrplatformloader.so", RTLD_NOLOAD);
  if (!ovrplatformloader_handle) {
    LOG_ERROR("could not dlopen libovrplatformloader.so: {}", dlerror());
    return nullptr;
  }
  return ovrplatformloader_handle;
}

uint32_t* get_ovrplatformloader_base() {
  if (ovrplatformloader_base) return ovrplatformloader_base;
  auto handle = get_ovrplatformloader_handle();
  if (!handle) {
    LOG_ERROR("libovrplatformloader.so handle was not found! can't get the base address");
    return nullptr;
  }

  ovrplatformloader_base = (uint32_t*)Util::baseAddr("libovrplatformloader.so");
  if (!ovrplatformloader_base) {
    LOG_ERROR("could not get the base address for libovrplatformloader.so");
    return nullptr;
  }

  return ovrplatformloader_base;
}

static constexpr auto ret_ins = 0xD65F03C0;
void hook_ovr_Message_GetUserArray(uint32_t* target) {
  Util::protect(target, PROT_READ | PROT_WRITE | PROT_EXEC);
  *target = ret_ins;
  Util::protect(target, PROT_READ | PROT_EXEC);
}

void hook_ovr_Message_GetDestinationArray(uint32_t* target) {
  Util::protect(target, PROT_READ | PROT_WRITE | PROT_EXEC);
  *target = ret_ins;
  Util::protect(target, PROT_READ | PROT_EXEC);
}

void hook_ovr_Message_GetPurchaseArray(uint32_t* target) {
  Util::protect(target, PROT_READ | PROT_WRITE | PROT_EXEC);
  *target = ret_ins;
  Util::protect(target, PROT_READ | PROT_EXEC);
}

void hook_ovr_Message_GetProductArray(uint32_t* target) {
  Util::protect(target, PROT_READ | PROT_WRITE | PROT_EXEC);
  *target = ret_ins;
  Util::protect(target, PROT_READ | PROT_EXEC);
}

void hook_ovr_Message_GetRoomInviteNotificationArray(uint32_t* target) {
  Util::protect(target, PROT_READ | PROT_WRITE | PROT_EXEC);
  *target = ret_ins;
  Util::protect(target, PROT_READ | PROT_EXEC);
}

#define HOOK(method)                                          \
  do {                                                        \
    auto addr = (uint32_t*)dlsym(handle, #method);            \
    if (!addr) {                                              \
      LOG_ERROR("Could not hook " #method ": {}", dlerror()); \
      break;                                                  \
    }                                                         \
    hook_##method(addr);                                      \
  } while (0)

void install_ovr_hooks() {
  dlerror();
  auto handle = get_ovrplatformloader_handle();
  if (!handle) return;

  HOOK(ovr_Message_GetUserArray);
  HOOK(ovr_Message_GetDestinationArray);
  HOOK(ovr_Message_GetPurchaseArray);
  HOOK(ovr_Message_GetProductArray);
  HOOK(ovr_Message_GetRoomInviteNotificationArray);
}
