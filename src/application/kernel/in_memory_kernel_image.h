/***
 * @file in_memory_kernel_image.h
 * @brief Deterministic in-memory kernel image for tests
 * @author Amaury Carvalho (2026)
 */

#ifndef IN_MEMORY_KERNEL_IMAGE_H_INCLUDED
#define IN_MEMORY_KERNEL_IMAGE_H_INCLUDED

#include <vector>

#include "kernel_image.h"

class InMemoryKernelImage : public KernelImage {
 private:
  std::vector<unsigned char> kernelBytes;
  std::vector<unsigned char> startCode;

 public:
  InMemoryKernelImage() {}
  InMemoryKernelImage(const std::vector<unsigned char>& bytes,
                      const std::vector<unsigned char>& start = std::vector<unsigned char>());

  void setBytes(const std::vector<unsigned char>& bytes);
  void setStartBytes(const std::vector<unsigned char>& bytes);

  const unsigned char* bytes() const override;
  std::size_t size() const override;
  const unsigned char* startBytes() const override;
  std::size_t startSize() const override;

  unsigned int dispatchAddress(int index) const override;
  unsigned int resolveCall(unsigned int address) const override;
};

#endif  // IN_MEMORY_KERNEL_IMAGE_H_INCLUDED
