/***
 * @file in_memory_kernel_image.cpp
 * @brief Deterministic in-memory kernel image implementation
 * @author Amaury Carvalho (2026)
 */

#include "in_memory_kernel_image.h"

InMemoryKernelImage::InMemoryKernelImage(
    const std::vector<unsigned char>& bytes,
    const std::vector<unsigned char>& start)
    : kernelBytes(bytes), startCode(start) {}

void InMemoryKernelImage::setBytes(
    const std::vector<unsigned char>& bytes) {
  kernelBytes = bytes;
}

void InMemoryKernelImage::setStartBytes(
    const std::vector<unsigned char>& bytes) {
  startCode = bytes;
}

const unsigned char* InMemoryKernelImage::bytes() const {
  return kernelBytes.empty() ? nullptr : kernelBytes.data();
}

std::size_t InMemoryKernelImage::size() const {
  return kernelBytes.size();
}

const unsigned char* InMemoryKernelImage::startBytes() const {
  return startCode.empty() ? nullptr : startCode.data();
}

std::size_t InMemoryKernelImage::startSize() const {
  return startCode.size();
}

unsigned int InMemoryKernelImage::dispatchAddress(int index) const {
  std::size_t offset = static_cast<std::size_t>(index) * 2;
  if (offset + 1 >= kernelBytes.size()) return 0;
  return kernelBytes[offset] | (kernelBytes[offset + 1] << 8);
}

unsigned int InMemoryKernelImage::resolveCall(unsigned int address) const {
  return resolveKernelCallAddress(bytes(), size(), address);
}
