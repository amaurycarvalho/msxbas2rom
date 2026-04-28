/***
 * @file for_next_node.h
 * @brief ForNextNode class header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef FOR_NEXT_NODE_H_INCLUDED
#define FOR_NEXT_NODE_H_INCLUDED

#include <memory>

class TagNode;
class Lexeme;
class FixNode;
class SymbolNode;
class ActionNode;

/***
 * @class ForNextNode
 * @brief FOR/NEXT address to be calculated during linking phase
 */
class ForNextNode {
 public:
  int index;
  shared_ptr<TagNode> tag;
  shared_ptr<Lexeme> for_var, for_to, for_step;
  shared_ptr<FixNode> forEndMark;
  shared_ptr<SymbolNode> forStepMark;
  shared_ptr<ActionNode> for_to_action, for_step_action;
};

#endif  // FOR_NEXT_NODE_H_INCLUDED
