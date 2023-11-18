#include "EchoRewindPatches.hpp"
#include <algorithm>
#include "echo-utils/shared/echo-utils.hpp"
#include "log.h"

void EchoRewindPatches::apply_patches(uintptr_t base) {
  for (auto const& [offset, instructions] : patches) {
    auto start = (uint32_t*)(base + offset);
    LOG_INFO("Writing {} bytes at address {}", instructions.size() * sizeof(uint32_t), fmt::ptr(start));

    if (!EchoUtils::protect(start, EchoUtils::RWX)) {
      LOG_ERROR("Could not apply patch at {} because protection failed", fmt::ptr(start));
      continue;
    }

    std::memcpy(start, instructions.data(), instructions.size() * sizeof(uint32_t));
    EchoUtils::protect(start, EchoUtils::RX);
  }
}
