/***
 * @file action_node.h
 * @brief ActionNode class header
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#ifndef ACTION_NODE_H_INCLUDED
#define ACTION_NODE_H_INCLUDED

#include <memory>
#include <string>
#include <vector>

class Lexeme;

using namespace std;

/***
 * @class ActionNode
 * @brief It represents a simple action item (statement, function etc)
 */
class ActionNode {
 private:
  void create(shared_ptr<Lexeme> plexeme);

 public:
  int subtype;
  shared_ptr<Lexeme> lexeme;
  vector<shared_ptr<ActionNode>> actions;
  string toString(int indent = 0) const;

  /***
   * @brief ActionNode class constructor.
   * It represents a simple action item (statement, function etc)
   */
  ActionNode();
  ~ActionNode();

  /***
   * @brief ActionNode class constructor.
   * It represents a simple action item (statement, function etc)
   * @param plexeme Action node lexeme item
   */
  ActionNode(shared_ptr<Lexeme> plexeme);

  /***
   * @brief ActionNode class constructor.
   * It represents a simple action item (statement, function etc)
   * @param name Keyword name for the new lexeme to be associated with this
   * action node
   */
  ActionNode(string name);
};

#endif  // ACTION_NODE_H_INCLUDED
