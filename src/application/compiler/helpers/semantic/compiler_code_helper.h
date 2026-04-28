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

  /***
   * @brief Enable ROM BASIC slot on page 1
   */
  void addEnableBasicSlot();

  /***
   * @brief Disable ROM BASIC slot on page 1
   */
  void addDisableBasicSlot();

  /***
   * @brief Disk BASIC (BDOS) calling routine without error handling
   * @note requires addEnableBasicSlot() and addDisableBasicSlot()
   */
  void addCallBDOS();

  /***
   * @brief Disk BASIC (BDOS) calling routine with error handling
   * @note requires addEnableBasicSlot() and addDisableBasicSlot()
   */
  void addCallBDOSWE();

  /***
   * @brief SET statement line writer
   * @note use it to build SET command lines at runtime
   */
  void beginBasicSetStmt(string name);
  void endBasicSetStmt();
  void addBasicChar(char c);

  CompilerCodeHelper(shared_ptr<CompilerContext> context);
  ~CompilerCodeHelper();
};

#endif  // COMPILER_CODE_HELPER_H