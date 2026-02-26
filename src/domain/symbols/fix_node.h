/***
 * @file fix_node.h
 * @brief FixNode class header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef FIX_NODE_H_INCLUDED
#define FIX_NODE_H_INCLUDED

#include "symbol_node.h"

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

#endif  // FIX_NODE_H_INCLUDED
