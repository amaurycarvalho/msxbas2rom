/***
 * @file tag_node.h
 * @brief TagNode class header
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#ifndef TAG_NODE_H_INCLUDED
#define TAG_NODE_H_INCLUDED

#include <memory>
#include <string>
#include <vector>

class ActionNode;
class LexerLineContext;

using namespace std;

/***
 * @class TagNode
 * @brief It represents a tag node to an action list
 */
class TagNode {
 public:
  string name;
  string value;
  vector<shared_ptr<ActionNode>> actions;
  shared_ptr<LexerLineContext> lexerLine;

  string toString() const;

  TagNode();
  ~TagNode();
};

#endif  // TAG_NODE_H_INCLUDED
