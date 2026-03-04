/***
 * @file cpu_workspace_context.h
 * @brief Shared mutable CPU workspace state for compiler/writer pipeline
 */

#ifndef CPU_WORKSPACE_CONTEXT_H
#define CPU_WORKSPACE_CONTEXT_H

#include <deque>
#include <memory>

using namespace std;

class CpuWorkspaceContext {
 public:
  CpuWorkspaceContext(int codeSizeLimit, int ramSizeLimit, int ramStartAddress)
      : code_size_limit(codeSizeLimit),
        ram_size_limit(ramSizeLimit),
        ram_start_address(ramStartAddress),
        code(new unsigned char[codeSizeLimit]),
        ram(new unsigned char[ramSizeLimit]) {
    clear();
  };
  ~CpuWorkspaceContext();

  void clear();

  int code_size_limit;
  int ram_size_limit;
  int ram_start_address;

  unique_ptr<unsigned char[]> code;
  int code_start;
  int code_pointer;
  int code_size;

  unique_ptr<unsigned char[]> ram;
  int ram_start;
  int ram_pointer;
  int ram_size;
  int ram_page;
  float ram_memory_perc;

  int segm_last;
  int segm_total;
  deque<unsigned char*> code_pipeline;
};

#endif  // CPU_WORKSPACE_CONTEXT_H
