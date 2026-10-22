/***
 * @file kernel_image.cpp
 * @brief Read-only application kernel image port
 * @author Amaury Carvalho (2026)
 */

#include "kernel_image.h"

#include "compiler_hooks.h"

unsigned int resolveKernelCallAddress(const unsigned char* bytes,
                                      std::size_t size,
                                      unsigned int address) {
  if (!bytes) return address;

  if (address < def_wrapper_routines_map_table + DISP_ENTRIES * 2) {
    if (address + 1 < size) {
      return bytes[address] | (bytes[address + 1] << 8);
    }
  } else if (address >= 0x4000 && address < 0x8000) {
    if (address + 2 < size && bytes[address] == 0xC3) {
      return bytes[address + 1] | (bytes[address + 2] << 8);
    }
  }

  return address;
}
