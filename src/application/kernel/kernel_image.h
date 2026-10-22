/***
 * @file kernel_image.h
 * @brief Read-only application kernel image port
 * @author Amaury Carvalho (2026)
 */

#ifndef KERNEL_IMAGE_H_INCLUDED
#define KERNEL_IMAGE_H_INCLUDED

#include <cstddef>
#include <memory>

/***
 * @brief Immutable kernel bytes and dispatch entry resolution.
 */
class KernelImage {
 public:
  virtual ~KernelImage() {}

  virtual const unsigned char* bytes() const = 0;
  virtual std::size_t size() const = 0;

  virtual const unsigned char* startBytes() const = 0;
  virtual std::size_t startSize() const = 0;

  /***
   * @brief Resolve a wrapper-routine dispatch table index to a kernel address.
   */
  virtual unsigned int dispatchAddress(int index) const = 0;

  /***
   * @brief Resolve a raw kernel call/jump address through the dispatch map.
   */
  virtual unsigned int resolveCall(unsigned int address) const = 0;
};

/***
 * @brief Shared dispatch-resolution algorithm.
 */
unsigned int resolveKernelCallAddress(const unsigned char* bytes,
                                      std::size_t size,
                                      unsigned int address);

#endif  // KERNEL_IMAGE_H_INCLUDED
