#pragma once

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace EchoPatchApplicator {
struct Patch {
  uint32_t offset;
  std::vector<uint8_t> bytes;
};

struct PatchSet {
  std::string lib;
  std::vector<Patch> patches;
};

struct PatchFile {
  std::filesystem::path path;
  std::vector<PatchSet> patchSets;
  static PatchFile read(std::filesystem::path filepath);
  void write(std::filesystem::path filepath);
};
}  // namespace EchoPatchApplicator
