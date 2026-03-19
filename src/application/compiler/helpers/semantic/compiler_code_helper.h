/***
 * @file compiler_code_helper.h
 * @brief Compiler code helper
 */

#ifndef COMPILER_CODE_HELPER_H
#define COMPILER_CODE_HELPER_H

#include <memory>
#include <string>

class CompilerContext;

using namespace std;

class CompilerCodeHelper {
 private:
  shared_ptr<CompilerContext> context;

 public:
  bool addCheckTraps();
  void addEnableBasicSlot();
  void addDisableBasicSlot();

  void beginBasicSetStmt(string name);
  void endBasicSetStmt();
  void addBasicChar(char c);

  CompilerCodeHelper(shared_ptr<CompilerContext> context);
  ~CompilerCodeHelper();
};

#endif  // COMPILER_CODE_HELPER_H