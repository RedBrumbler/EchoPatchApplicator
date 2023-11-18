# EchoPatchApplicator

Applies patches placed in `/sdcard/ModData/com.readyatdawn.r15/Mods/PatchApplicator/` to the game at runtime.
Thanks to the work on [EchoRewind](https://github.com/C-Luddy/EchoRewind) the patches in `rewind.patch` were made.

# Patch File
 A patch file is a fairly simple file structure created to simplify making them. they are not compressed as the kind of patches you'd usually apply are fairly simple

The structure is more or less as follows:
The file starts with an uint32 which is the amount of patch sets in the file, this is a set per library.
Each set starts with the length of the libname as an uint32, after which the library name immediately follows.
Then a uint32 follows again, which stores the amount of patches for that lib.
Then for each of those patches, an offset is stored as uint32, after which a uint32 is stored for the amount of bytes (uint8) to apply at that offset.

This results in more or less this:

```
PatchFile
L uint32 patch set count
  L uint32 libname length
  L libname
  L uint32 patch count
  | L uint32 offset
  | L uint32 byte count
  | | L uint8[] bytes
  | L ...
  L...
```

## Installation

Install the [yodel](https://github.com/RedBrumbler/Yodel) modloader and drop the .so file from this mod into the `/sdcard/ModData/com.readyatdawn.r15/Modloader/early_mods/` folder
