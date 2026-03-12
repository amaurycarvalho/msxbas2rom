/***
 * @file tag_node.cpp
 * @brief TagNode class implementation
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#include "tag_node.h"

#include "action_node.h"
#include "lexer_line.h"

string TagNode::toString() const {
  string out = "";

  if (lexerLine) out = lexerLine->lineText;

  if (!out.empty())
    if (out.back() != '\n' && out.back() != '\r') out += "\n";

  out += "Tag ";
  out += name;
  out += "\n";

  for (unsigned int i = 0; i < actions.size(); i++) {
    out += actions[i]->toString(0);
  }

  return out;
}

TagNode::TagNode() {
  lexerLine = nullptr;
}

TagNode::~TagNode() = default;
