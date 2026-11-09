# Localiser

The `shared_fw` PlatformIO project produces two separate firmware binaries:

- `platformio run -e anchor` builds `src/anchor/main.cpp`.
- `platformio run -e target` builds `src/target/main.cpp`.

ESP-IDF does not support PlatformIO source filters, so `src/CMakeLists.txt`
registers both role sources. The environment-specific `IS_ANCHOR` and
`IS_TARGET` defines ensure only the selected entry point contributes code to
each binary.
