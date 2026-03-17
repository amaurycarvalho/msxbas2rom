/***
 * @file code_node.h
 * @brief CodeNode class header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef CODE_NODE_H_INCLUDED
#define CODE_NODE_H_INCLUDED

#include <memory>
#include <string>

class Lexeme;

using namespace std;

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
  int lineNumber = 0;
  shared_ptr<Lexeme> lexeme;
};

#endif  // CODE_NODE_H_INCLUDED
