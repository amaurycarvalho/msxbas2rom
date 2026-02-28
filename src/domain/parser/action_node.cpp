/***
 * @file action_node.cpp
 * @brief ActionNode class implementation
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#include "action_node.h"

ActionNode::ActionNode() {
  create((Lexeme*)0);
}

ActionNode::ActionNode(string name) {
  create(new Lexeme(Lexeme::type_keyword, Lexeme::subtype_any, name));
}

ActionNode::ActionNode(Lexeme* plexeme) {
  create(plexeme);
}

void ActionNode::create(Lexeme* plexeme) {
  lexeme = plexeme;
  subtype = Lexeme::subtype_unknown;
  actions.clear();
}
