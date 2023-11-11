#include "log.h"

#include <dlfcn.h>
#include <sys/mman.h>
#include "echo-utils/shared/echo-utils.hpp"
#include "flamingo/shared/trampoline-allocator.hpp"
#include "flamingo/shared/trampoline.hpp"
#include "yodel/shared/modloader.h"

#include "log.h"

#include <pthread.h>

struct CEnvironment {};
struct CMemParms {
  uint64_t size;
  uint64_t capacity;
  void* allocator;
};
static_assert(sizeof(CMemParms) == 0x18);
struct CMemBlock {
  void* memory;
  uint64_t capacity;
  void* allocator;
  uint32_t size;
  uint32_t flags;
};
static_assert(sizeof(CMemBlock) == 0x20);
struct CString : public CMemBlock {
  uint64_t num_1;
  uint64_t num_2;
  uint64_t num_3;
};
static_assert(sizeof(CString) == 0x38);

namespace CallbackPatch {

using namespace std::string_view_literals;

#define SYM_METHOD(lib, symbol, ret, name, ...)                                                                  \
  ret(*name) __VA_ARGS__;                                                                                        \
  void find_##name() {                                                                                           \
    static auto lookup = EchoUtils::HandleUtils::get_symbol_address(lib, symbol);                                \
    name = reinterpret_cast<decltype(name)>(lookup.first);                                                       \
    if (!name)                                                                                                   \
      LOG_ERROR("Could not find symbol '{}' in library '{}': {} for method " #name, symbol, lib, lookup.second); \
  }

#define INIT_SYM_METHOD(name) find_##name()

int* tls;
CEnvironment** gCEnvironment;

SYM_METHOD("libpnsovr.so", "_ZN10NRadEngine12CEnvironment12GetTlsHandleENS_9CSymbol64Ey", int, GetTlsHandle,
           (CEnvironment * self, uint8_t param_2, uint64_t param_3));

SYM_METHOD("libpnsovr.so", "_ZN10NRadEngine10CSysThread6GetTLSEi", void*, GetTLS, (int key));
SYM_METHOD("libpnsovr.so", "_ZN10NRadEngine10CSysThread6SetTLSEiPv", int, SetTLS, (int key, void* value));
SYM_METHOD("libpnsovr.so", "_ZNK10NRadEngine12CEnvironment9GetGlobalENS_9CSymbol64Ey", void*, GetGlobal,
           (CEnvironment * self, uint8_t param_2, uint64_t param_3));
SYM_METHOD("libpnsovr.so", "_ZN10NRadEngine7CStringC2EPKcjRKNS_9CMemParmsE", void, CString_ctor,
           (CString * self, char const* content, uint param_2, CMemParms* parms));
SYM_METHOD("libpnsovr.so", "_ZN10NRadEngine7CStringaSEOS0_", CString*, CString_assign,
           (CString * self, CString* other));
SYM_METHOD("libpnsovr.so", "_ZN10NRadEngine14CMemoryContext10CurrentPtrEv", void*, CurrentPtr, ());

void perform_symbol_lookups() {
  gCEnvironment = (CEnvironment**)EchoUtils::AddressUtils::get_offset("libpnsovr.so", 0x70b410);
  tls = (int*)EchoUtils::AddressUtils::get_offset("libpnsovr.so", 0x6e5158);

  INIT_SYM_METHOD(GetTlsHandle);
  INIT_SYM_METHOD(GetTLS);
  INIT_SYM_METHOD(SetTLS);
  INIT_SYM_METHOD(GetGlobal);
  INIT_SYM_METHOD(CString_ctor);
  INIT_SYM_METHOD(CString_assign);
  INIT_SYM_METHOD(CurrentPtr);
}

void install_access_token_hook() {
  perform_symbol_lookups();

  auto lookup = EchoUtils::HandleUtils::get_symbol_address(
      "libpnsovr.so", "_ZN10NRadEngine10SCallbacks28GotLoggedInUserAccessTokenCbEP10ovrMessage");
  auto target = (uint32_t*)lookup.first;
  if (!target) {
    LOG_ERROR("Could not find _ZN10NRadEngine10SCallbacks28GotLoggedInUserAccessTokenCbEP10ovrMessage in pnsovr: {}",
              lookup.second);
    return;
  }

  if (!EchoUtils::protect(target, EchoUtils::RWX)) return;
  static auto trampoline = flamingo::TrampolineAllocator::Allocate(128);
  trampoline.WriteHookFixups(target);
  trampoline.WriteCallback(&target[4]);
  trampoline.Finish();

  static auto cb_hook = [](void* self, void* message) {
    LOG_INFO("Stopped errors being emitted from access token hook, providing fake access token DEADBEEFDEADBEEFDEAD");

    if (*tls == -1) {
      *tls = GetTlsHandle(*gCEnvironment, 0xf8, 0);
    }

    auto stored = reinterpret_cast<CString*>(GetTLS(*tls));
    if (!stored) {
      stored = reinterpret_cast<CString*>(GetGlobal(*gCEnvironment, 0xf8, 0));
      SetTLS(*tls, stored);
    }

    CString str;
    CMemParms parms{ .size = 0, .capacity = 0x20, .allocator = CurrentPtr() };
    CString_ctor(&str, "DEADBEEFDEADBEEFDEAD", 0, &parms);
    CString_assign(stored, &str);
  };

  std::size_t ins_count = 8;
  std::size_t trampoline_size = 64;
  auto target_hook = flamingo::Trampoline(target, ins_count, trampoline_size);
  target_hook.WriteCallback(reinterpret_cast<uint32_t*>(+cb_hook));
  target_hook.Finish();

  EchoUtils::protect(target, EchoUtils::RX);
}

}  // namespace CallbackPatch
