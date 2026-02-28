/***
 * @file tag_node.cpp
 * @brief TagNode class implementation
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#include "tag_node.h"

string TagNode::toString() const {
  string out = "Tag ";
  out += name;
  out += "\n";

  for (unsigned int i = 0; i < actions.size(); i++) {
    out += actions[i]->toString(0);
  }

  return out;
}
