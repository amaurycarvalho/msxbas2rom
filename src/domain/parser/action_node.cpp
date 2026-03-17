/***
 * @file action_node.cpp
 * @brief ActionNode class implementation
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#include "action_node.h"

#include "lexeme.h"

ActionNode::ActionNode() {
  create(make_shared<Lexeme>());
}

ActionNode::ActionNode(string name) {
  create(make_shared<Lexeme>(Lexeme::type_keyword, Lexeme::subtype_any, name));
}

ActionNode::ActionNode(shared_ptr<Lexeme> plexeme) {
  create(plexeme);
}

ActionNode::~ActionNode() = default;

void ActionNode::create(shared_ptr<Lexeme> plexeme) {
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

    for (auto& action : actions) {
      out += action->toString(indent + 2);
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
