/***
 * @file cpu_workspace_context.cpp
 * @brief Shared mutable CPU workspace state for compiler/writer pipeline
 */

#include "cpu_workspace_context.h"

#include <cstdlib>
#include <cstring>

void CpuWorkspaceContext::clear() {
  code_start = 0;
  code_pointer = code_start;
  code_size = 0;
  memset(code.get(), 0x00, code_size_limit);

  ram_size = 0;
  ram_page = 0x8000;
  ram_start = ram_start_address - ram_page;
  ram_pointer = ram_start;
  ram_memory_perc = 0;
  memset(ram.get(), 0x00, ram_size_limit);

  segm_last = 0;
  segm_total = 0;

  //! @remark
  //! initialize the pipeline with "five NOP instructions"
  //! as a safe buffer to code optimizing logic
  code_pipeline.clear();
  code_pipeline.push_back(&code[code_size_limit - 5]);
  code_pipeline.push_back(code_pipeline[0]);
  code_pipeline.push_back(code_pipeline[0]);
  code_pipeline.push_back(code_pipeline[0]);
  code_pipeline.push_back(code_pipeline[0]);
}

CpuWorkspaceContext::~CpuWorkspaceContext() {}
