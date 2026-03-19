/***
 * @file compiler_code_optimizer.h
 * @brief Compiler code optimizer
 */

#ifndef COMPILER_CODE_OPTIMIZER_H
#define COMPILER_CODE_OPTIMIZER_H

#include <memory>

class CompilerContext;

using namespace std;

class CompilerCodeOptimizer {
 private:
  shared_ptr<CompilerContext> context;

 public:
  void addByteOptimized(unsigned char byte);
  void addKernelCall(unsigned int word);
  int getKernelCallAddr(unsigned int word);
  void addLdHLmegarom();

  CompilerCodeOptimizer(shared_ptr<CompilerContext> context);
  ~CompilerCodeOptimizer();
};

#endif  // COMPILER_CODE_OPTIMIZER_H