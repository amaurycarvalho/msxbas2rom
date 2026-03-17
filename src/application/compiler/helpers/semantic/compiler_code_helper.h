/***
 * @file compiler_code_helper.h
 * @brief Compiler code helper
 */

#ifndef COMPILER_CODE_HELPER_H
#define COMPILER_CODE_HELPER_H

#include <string>

class CompilerContext;

using namespace std;

class CompilerCodeHelper {
 private:
  CompilerContext* context;

 public:
  bool addCheckTraps();
  void addEnableBasicSlot();
  void addDisableBasicSlot();

  void beginBasicSetStmt(string name);
  void endBasicSetStmt();
  void addBasicChar(char c);

  CompilerCodeHelper(CompilerContext* context);
  ~CompilerCodeHelper();
};

#endif  // COMPILER_CODE_HELPER_H