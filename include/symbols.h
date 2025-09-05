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
#include "lex.h"
#include "parse.h"
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
 * @class FileNode
 * @brief File wrapper for the file system
 */
class FileNode {
 private:
  unsigned char* s;
  FILE* handle;
  Pletter pletter;
  bool first;
  int bytes;
  unsigned char buf_plain[255], buf_packed[1024];

 public:
  string name;
  int length;
  bool packed;
  int packed_length;
  int blocks;
  Lexeme* current_lexeme;
  Lexeme* first_lexeme;
  unsigned char* buffer;

  unsigned char file_header[255];

  ~FileNode();

  bool create();
  bool open();
  bool eof();
  int read(unsigned char* data, int max_length);
  void write(unsigned char* data, int data_length);
  void close();
  void clear();

  int read();
  int readAsLexeme();
  int readAsLexeme(unsigned char* data, int data_length);

  /// @brief strips quotes from a string
  string stripQuotes(const string& text);
  /// @brief string to uppercase
  string toUpper(const string& input);
  /// @brief get file extension in uppercase
  string getFileExt(string filename);
  string getFileExt();

  bool writeToFile(string filename, unsigned char* data, int data_length);
  int readFromFile(string filename, unsigned char* data, int maxlen);
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