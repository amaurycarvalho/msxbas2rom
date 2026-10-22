/***
 * @file generated_kernel_image.cpp
 * @brief Generated kernel adapter implementation
 * @author Amaury Carvalho (2026)
 */

#include "generated_kernel_image.h"

#include "header.h"
#include "start.h"

const unsigned char* GeneratedKernelImage::bytes() const {
  return bin_header_bin;
}

std::size_t GeneratedKernelImage::size() const {
  return bin_header_bin_len;
}

const unsigned char* GeneratedKernelImage::startBytes() const {
  return bin_start_bin;
}

std::size_t GeneratedKernelImage::startSize() const {
  return bin_start_bin_len;
}

unsigned int GeneratedKernelImage::dispatchAddress(int index) const {
  std::size_t offset = static_cast<std::size_t>(index) * 2;
  if (offset + 1 >= size()) return 0;
  return bin_header_bin[offset] | (bin_header_bin[offset + 1] << 8);
}

unsigned int GeneratedKernelImage::resolveCall(unsigned int address) const {
  return resolveKernelCallAddress(bytes(), size(), address);
}
