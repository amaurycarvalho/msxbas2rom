/***
 * @file generated_kernel_image.h
 * @brief Generated kernel adapter
 * @author Amaury Carvalho (2026)
 */

#ifndef GENERATED_KERNEL_IMAGE_H_INCLUDED
#define GENERATED_KERNEL_IMAGE_H_INCLUDED

#include "kernel_image.h"

class GeneratedKernelImage : public KernelImage {
 public:
  GeneratedKernelImage() {}

  const unsigned char* bytes() const override;
  std::size_t size() const override;
  const unsigned char* startBytes() const override;
  std::size_t startSize() const override;
  unsigned int dispatchAddress(int index) const override;
  unsigned int resolveCall(unsigned int address) const override;
};

#endif  // GENERATED_KERNEL_IMAGE_H_INCLUDED
