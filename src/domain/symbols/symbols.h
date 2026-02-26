/***
 * @file symbols.h
 * @brief Symbols class header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef SYMBOLS_H_INCLUDED
#define SYMBOLS_H_INCLUDED

#include <string.h>

#include <queue>

#include "fswrapper.h"
#include "lexer.h"
#include "parser.h"
#include "pletter.h"

/***
 * @class SymbolNode
 * @brief Symbol address for a tag node
 */
class SymbolNode {
 public:
  Lexeme* lexeme;
  TagNode* tag;
  int address;
};

/***
 * @class FixNode
 * @brief Symbol address to be calculated during linking phase
 */
class FixNode {
 public:
  SymbolNode* symbol;
  int address;
  int step;
};

/***
 * @class ForNextNode
 * @brief FOR/NEXT address to be calculated during linking phase
 */
class ForNextNode {
 public:
  int index;
  TagNode* tag;
  Lexeme *for_var, *for_to, *for_step;
  FixNode* for_end_mark;
  SymbolNode* for_step_mark;
  ActionNode *for_to_action, *for_step_action;
};

/***
 * @class CodeNode
 * @brief Compiled code for a tag node
 */
class CodeNode {
 public:
  string name;
  int start;
  int length;
  int segm;
  int addr_within_segm;
  bool is_code;
  bool debug;
};

/***
 * @class SymbolManager
 * @brief Symbol manager
 */
class SymbolManager {
 private:
  vector<vector<string>> getKernelSymbolAddresses();

 public:
  vector<CodeNode*> codeList;
  vector<CodeNode*> dataList;

  /***
   * @brief clear all symbol lists
   */
  void clear();

  /***
   * @brief Write symbols file (*.symbol) to use with old/new OpenMSX Debugger
   * @param opts compiler options
   * @note
   * [old
   * debugger](https://github.com/openMSX/debugger/blob/master/src/SymbolTable.cpp)
   * ; [new
   * debugger](https://github.com/openMSX/openMSX/blob/master/src/debugger/SymbolManager.cc)
   * ; [new debugger unit
   * test](https://github.com/openMSX/openMSX/blob/master/src/unittest/SymbolManager_test.cc)
   * @remark depends on compile->write and compile->save_symbols methods to
   * show the correct symbol addresses
   */
  bool saveSymbolFile(BuildOptions* opts);

  /***
   * @brief Write NoICE file (*.noi) to use with new OpenMSX Debugger
   * @param opts compiler options
   * @note
   * [old
   * debugger](https://github.com/openMSX/debugger/blob/master/src/SymbolTable.cpp)
   * ; [new
   * debugger](https://github.com/openMSX/openMSX/blob/master/src/debugger/SymbolManager.cc)
   * ; [new debugger unit
   * test](https://github.com/openMSX/openMSX/blob/master/src/unittest/SymbolManager_test.cc)
   * @remark depends on compile->write and compile->save_symbols methods to
   * show the correct symbol addresses
   */
  bool saveNoIceFile(BuildOptions* opts);

  /***
   * @brief Write OMDS file (*.omds) to use with old OpenMSX Debugger
   * @param opts compiler options
   * @note
   * [old
   * debugger](https://github.com/openMSX/debugger/blob/master/src/SymbolTable.cpp)
   * ; [new
   * debugger](https://github.com/openMSX/openMSX/blob/master/src/debugger/SymbolManager.cc)
   * ; [new debugger unit
   * test](https://github.com/openMSX/openMSX/blob/master/src/unittest/SymbolManager_test.cc)
   * @remark depends on compile->write and compile->save_symbols methods to
   * show the correct symbol addresses
   */
  bool saveOmdsFile(BuildOptions* opts);
};

#endif  // SYMBOLS_H_INCLUDED