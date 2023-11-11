#include "flamingo/shared/trampoline-allocator.hpp"
#include "flamingo/shared/trampoline.hpp"
#include "yodel/shared/loader.hpp"

#include "log.h"
#include "patches.hpp"

extern "C" void setup(CModInfo* info) {
  info->id = MOD_ID;
  info->version = VERSION;
  info->version_long = GIT_COMMIT;
}

extern "C" void load() {
  LOG_INFO("Patching echovr to work offline from branch " GIT_BRANCH " (0x{:X}) by " GIT_USER, GIT_COMMIT);

  install_csysmodule_load_hook();
}
