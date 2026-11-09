# Localiser

The `shared_fw` PlatformIO project produces two separate firmware binaries:

- `platformio run -e anchor` builds `src/anchor/main.cpp`.
- `platformio run -e target` builds `src/target/main.cpp`.

ESP-IDF does not support PlatformIO source filters, so `src/CMakeLists.txt`
registers both role sources. The environment-specific `IS_ANCHOR` and
`IS_TARGET` defines ensure only the selected entry point contributes code to
each binary.

The shared library exposes separate role classes:

```cpp
LpsFtmInitiator anchor(16, 2);          // frame count, burst period
LpsFtmResponder target("LPS_TARGET_1"); // open AP
LpsFtmResponder secured("LPS_TARGET_1", "password");
```

Call `begin()` on the selected object during `setup()`.
`LpsFtmInitiator::latestDistance()` returns `std::nullopt` when the last
measurement was invalid and a distance in meters otherwise. The project
enables C++17 in both PlatformIO environments for this API.
