/***
 * @file symbol_node.h
 * @brief SymbolNode class header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef SYMBOL_NODE_H_INCLUDED
#define SYMBOL_NODE_H_INCLUDED

#include "parser.h"

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

#endif  // SYMBOL_NODE_H_INCLUDED
