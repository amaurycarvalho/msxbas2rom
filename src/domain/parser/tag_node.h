/***
 * @file tag_node.h
 * @brief TagNode class header
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#ifndef TAG_NODE_H_INCLUDED
#define TAG_NODE_H_INCLUDED

#include <string>
#include <vector>

#include "action_node.h"

using namespace std;

/***
 * @class TagNode
 * @brief It represents a tag node to an action list
 */
class TagNode {
 public:
  string name;
  string value;
  vector<ActionNode*> actions;
};

#endif  // TAG_NODE_H_INCLUDED
