/***
 * @file action_node.h
 * @brief ActionNode class header
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#ifndef ACTION_NODE_H_INCLUDED
#define ACTION_NODE_H_INCLUDED

#include <string>
#include <vector>

#include "lexeme.h"

using namespace std;

/***
 * @class ActionNode
 * @brief It represents a simple action item (statement, function etc)
 */
class ActionNode {
 private:
  void create(Lexeme* plexeme);

 public:
  int subtype;
  Lexeme* lexeme;
  vector<ActionNode*> actions;
  string toString(int indent = 0) const;

  /***
   * @brief ActionNode class constructor.
   * It represents a simple action item (statement, function etc)
   */
  ActionNode();

  /***
   * @brief ActionNode class constructor.
   * It represents a simple action item (statement, function etc)
   * @param plexeme Action node lexeme item
   */
  ActionNode(Lexeme* plexeme);

  /***
   * @brief ActionNode class constructor.
   * It represents a simple action item (statement, function etc)
   * @param name Keyword name for the new lexeme to be associated with this
   * action node
   */
  ActionNode(string name);
};

#endif  // ACTION_NODE_H_INCLUDED
