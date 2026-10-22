/***
 * @file cpu_workspace_context.h
 * @brief Shared mutable CPU workspace state for compiler/writer pipeline
 */

#ifndef CPU_WORKSPACE_CONTEXT_H
#define CPU_WORKSPACE_CONTEXT_H

#include <deque>
#include <vector>

using namespace std;

class CpuWorkspaceContext {
 public:
  CpuWorkspaceContext(int codeSizeLimit, int ramSizeLimit, int ramStartAddress);

  void clear();

  int& getCodeSizeLimit() { return code_size_limit; }
  int& getRamSizeLimit() { return ram_size_limit; }
  int& getRamStartAddress() { return ram_start_address; }
  bool& getError() { return error; }
  vector<unsigned char>& getCode() { return code; }
  int& getCodeStart() { return code_start; }
  int& getCodePointer() { return code_pointer; }
  int& getCodeSize() { return code_size; }
  vector<unsigned char>& getRam() { return ram; }
  int& getRamStart() { return ram_start; }
  int& getRamPointer() { return ram_pointer; }
  int& getRamSize() { return ram_size; }
  int& getRamPage() { return ram_page; }
  float& getRamMemoryPerc() { return ram_memory_perc; }
  int& getSegmLast() { return segm_last; }
  int& getSegmTotal() { return segm_total; }
  deque<unsigned char*>& getCodePipeline() { return code_pipeline; }

 private:
  int code_size_limit;
  int ram_size_limit;
  int ram_start_address;

  bool error;

  vector<unsigned char> code;
  int code_start;
  int code_pointer;
  int code_size;

  vector<unsigned char> ram;
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
