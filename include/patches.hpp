#pragma once

namespace CallbackPatch {
void install_loggedincb_hook();
void install_access_token_hook();
}  // namespace CallbackPatch

extern void install_offline_patches();
extern void install_csysmodule_load_hook();
extern void install_ovr_hooks();
