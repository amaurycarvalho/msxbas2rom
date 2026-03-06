/***
 * @file compiler_code_optimizer.h
 * @brief Compiler code optimizer
 */

#ifndef COMPILER_CODE_OPTIMIZER_H
#define COMPILER_CODE_OPTIMIZER_H

class CompilerContext;

class CompilerCodeOptimizer {
 private:
  CompilerContext* context;

 public:
  void addByteOptimized(unsigned char byte);
  void addKernelCall(unsigned int word);
  int getKernelCallAddr(unsigned int word);
  void addLdHLmegarom();

  CompilerCodeOptimizer(CompilerContext* context) : context(context) {};
};

#endif  // COMPILER_CODE_OPTIMIZER_H