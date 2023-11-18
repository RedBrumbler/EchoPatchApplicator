#include "PatchFile.hpp"
#include <fstream>

namespace EchoPatchApplicator {
void PatchFile::write(std::filesystem::path filepath) {
  auto file = std::ofstream(filepath, std::ios::binary | std::ios::out);
  uint32_t setCount = patchSets.size();
  file.write(reinterpret_cast<char const*>(&setCount), sizeof(uint32_t));

  for (auto const& set : patchSets) {
    uint32_t libNameLength = set.lib.size();
    file.write(reinterpret_cast<char const*>(&libNameLength), sizeof(uint32_t));
    file.write(set.lib.c_str(), libNameLength);

    uint32_t patchCount = set.patches.size();
    file.write(reinterpret_cast<char const*>(&patchCount), sizeof(uint32_t));

    for (auto const& patch : set.patches) {
      file.write(reinterpret_cast<char const*>(&patch.offset), sizeof(uint32_t));

      uint32_t patchlen = patch.bytes.size();
      file.write(reinterpret_cast<char*>(&patchlen), sizeof(int));
      file.write(reinterpret_cast<char const*>(patch.bytes.data()), patchlen);
    }
  }
  file.close();
}

PatchFile PatchFile::read(std::filesystem::path filepath) {
  auto file = std::ifstream(filepath, std::ios::binary | std::ios::in | std::ios::ate);
  auto len = file.tellg();
  file.seekg(0);

  std::vector<PatchSet> patchSets;

  // first num: sets
  uint32_t setCount;
  file.read(reinterpret_cast<char*>(&setCount), sizeof(uint32_t));
  // read N sets
  for (uint32_t i = 0; i < setCount; i++) {
    // each set is a libname followed by an amount of patches for that lib

    uint32_t libNameLength;
    file.read(reinterpret_cast<char*>(&libNameLength), sizeof(uint32_t));

    // read libname
    std::string libName;
    libName.resize(libNameLength);
    file.read(libName.data(), libNameLength);

    std::vector<Patch> patches;
    // read patch count
    uint32_t patchCount;
    file.read(reinterpret_cast<char*>(&patchCount), sizeof(uint32_t));

    // read N patches
    for (uint32_t j = 0; j < patchCount; j++) {
      uint32_t offset;
      file.read(reinterpret_cast<char*>(&offset), sizeof(uint32_t));

      int patchlen;
      file.read(reinterpret_cast<char*>(&patchlen), sizeof(int));

      std::vector<uint8_t> bytes;
      bytes.resize(patchlen);
      file.read(reinterpret_cast<char*>(bytes.data()), patchlen);
      patches.push_back({ offset, bytes });
    }

    patchSets.push_back({ libName, patches });
  }

  file.close();
  return { filepath, patchSets };
}
}  // namespace EchoPatchApplicator
