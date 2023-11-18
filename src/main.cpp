#include "yodel/shared/loader.hpp"

#include "PatchFile.hpp"
#include "echo-utils/shared/echo-utils.hpp"
#include "log.h"

extern "C" void setup(CModInfo* info) {
  info->id = MOD_ID;
  info->version = VERSION;
  info->version_long = GIT_COMMIT;
}

std::vector<EchoPatchApplicator::PatchFile> get_patches(std::filesystem::path patchdir) {
  if (!std::filesystem::exists(patchdir)) {
    std::filesystem::create_directories(patchdir);
  }

  std::error_code error_code;
  std::filesystem::directory_iterator dir_iter(patchdir, error_code);
  if (error_code) {
    LOG_ERROR("Not applying any patches, error happened: {}", error_code.message().c_str());
  }

  std::vector<EchoPatchApplicator::PatchFile> files;

  for (auto const& file : dir_iter) {
    if (file.is_directory()) {
      continue;
    }
    if (file.path().extension() != ".patch") {
      continue;
    }

    LOG_DEBUG("Attempting load of patch file: {}", file.path().c_str());

    files.emplace_back(EchoPatchApplicator::PatchFile::read(file.path()));
  }

  return files;
}

void apply_patches(std::string const& soName, std::vector<EchoPatchApplicator::Patch> const& patches) {
  auto base = EchoUtils::AddressUtils::get_base(soName);
  if (!base) {
    LOG_WARN("Not applying {} patches to {}, as the base address could not be found", patches.size(), soName);
    return;
  }

  for (auto const& [offset, instructions] : patches) {
    auto start = (uint32_t*)(base + offset);
    LOG_INFO("Writing {} bytes at address {}", instructions.size() * sizeof(uint32_t), fmt::ptr(start));

    if (!EchoUtils::protect(start, EchoUtils::RWX)) {
      LOG_WARN("Could not apply patch at {} because protection failed", fmt::ptr(start));
      continue;
    }

    std::memcpy(start, instructions.data(), instructions.size());
    EchoUtils::protect(start, EchoUtils::RX);
  }
}

extern "C" void load() {
  LOG_INFO("Patching echovr to work from branch " GIT_BRANCH " (0x{:X})", GIT_COMMIT);

  auto patch_dir = fmt::format("/sdcard/ModData/{}/Mods/PatchApplicator", modloader::get_application_id());
  for (auto& file : get_patches(patch_dir)) {
    for (auto& [lib, patches] : file.patchSets) {
      // always starts with a /, so we skip
      auto soName = lib.substr(1);
      if (!soName.starts_with("lib")) {  // if not starting with lib now, add it
        soName = fmt::format("lib{}", soName);
      }
      if (!soName.ends_with(".so")) {  // if not ending with .so now, add it
        soName = fmt::format("{}.so", soName);
      }

      auto handle = EchoUtils::HandleUtils::get_handle_uncached(soName);
      if (handle) {
        apply_patches(soName, patches);
      } else {
        EchoUtils::Callbacks::add_library_callback(
            file.path.string(), lib, [soName = std::move(soName), patches = std::move(patches)](auto handle) {
              apply_patches(soName, patches);
            });
      }
    }
  }
}
