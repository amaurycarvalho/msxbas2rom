/***
 * @file kernel_central_table.h
 * @brief Runtime central jump table stub generation
 */

#ifndef KERNEL_CENTRAL_TABLE_H_INCLUDED
#define KERNEL_CENTRAL_TABLE_H_INCLUDED

#include <map>
#include <string>
#include <vector>

struct KernelDispatchStub {
  unsigned int dispatchIndex;
  bool paged;
  unsigned int address;
  unsigned int segment;
};

class KernelCentralTable {
 public:
  static const unsigned int residentStubSize = 3;
  static const unsigned int pagedStubSize = 6;

  static std::vector<unsigned char> residentStub(unsigned int target);
  static std::vector<unsigned char> pagedStub(unsigned int dispatcher,
                                              unsigned int segment,
                                              unsigned int target);

  static bool build(const std::vector<KernelDispatchStub>& stubs,
                    unsigned int dispatcherAddress, unsigned int baseAddress,
                    std::vector<unsigned char>& bytes,
                    std::map<unsigned int, unsigned int>& stubAddresses,
                    std::string& error);
};

#endif  // KERNEL_CENTRAL_TABLE_H_INCLUDED
