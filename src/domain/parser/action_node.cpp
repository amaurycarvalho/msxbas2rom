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

string ActionNode::toString(int indent) const {
  if (!lexeme) return "";

  string out;

  if (actions.size()) {
    out.append(indent + 2, ' ');
    out += "(\n";

    for (unsigned int i = 0; i < actions.size(); i++) {
      out += actions[i]->toString(indent + 2);
    }

    out.append(indent + 2, ' ');
    out += ") Action ";
    out += lexeme->value;
    out += "\n";

  } else {
    if (lexeme->type == Lexeme::type_keyword) {
      out.append(indent + 2, ' ');
      out += "Action ";
      out += lexeme->value;
      out += "\n";
    } else {
      out += lexeme->toString(indent);
    }
  }

  return out;
}
