/***
 * @file for_next_node.h
 * @brief ForNextNode class header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef FOR_NEXT_NODE_H_INCLUDED
#define FOR_NEXT_NODE_H_INCLUDED

#include "fix_node.h"

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

#endif  // FOR_NEXT_NODE_H_INCLUDED
