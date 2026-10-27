#ifndef COMPILER_TILE_PARAMETER_H_INCLUDED
#define COMPILER_TILE_PARAMETER_H_INCLUDED

#include <memory>
#include <string>

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_symbol_resolver.h"
#include "lexeme.h"
#include "symbol_node.h"

using namespace std;

inline shared_ptr<ActionNode> getDeclaredArrayIdentifier(
    shared_ptr<CompilerContext> context, shared_ptr<ActionNode> action) {
  if (!action || !action->lexeme) return nullptr;

  shared_ptr<Lexeme> lexeme = action->lexeme;

  if (lexeme->type == Lexeme::type_identifier && action->actions.empty()) {
    if (lexeme->isArray) return action;
    shared_ptr<SymbolNode> symbol =
        context->getSymbolResolver()->getSymbol(lexeme);
    if (symbol && symbol->lexeme && symbol->lexeme->isArray) return action;
    return nullptr;
  }

  if (lexeme->value == "ARRAY" && action->actions.size() == 1) {
    shared_ptr<ActionNode> child = action->actions[0];
    if (child && child->lexeme &&
        child->lexeme->type == Lexeme::type_identifier) {
      if (child->lexeme->isArray) return child;
      shared_ptr<SymbolNode> symbol =
          context->getSymbolResolver()->getSymbol(child->lexeme);
      if (symbol && symbol->lexeme && symbol->lexeme->isArray) return child;
    }
  }

  return nullptr;
}

inline bool isTileBufferAction(shared_ptr<CompilerContext> context,
                               shared_ptr<ActionNode> action) {
  return getDeclaredArrayIdentifier(context, action) != nullptr;
}

inline bool isTileTupleAction(shared_ptr<ActionNode> action) {
  return action && action->lexeme && action->lexeme->value == "ARRAY";
}

inline bool isTileArrayAction(shared_ptr<CompilerContext> context,
                              shared_ptr<ActionNode> action) {
  return isTileTupleAction(action) || isTileBufferAction(context, action);
}

inline bool getNumericLiteral(shared_ptr<ActionNode> action, int& value) {
  if (!action || !action->lexeme) return false;

  shared_ptr<Lexeme> lexeme = action->lexeme;
  if (lexeme->type != Lexeme::type_literal ||
      lexeme->subtype != Lexeme::subtype_numeric) {
    return false;
  }

  const string& text = lexeme->value;
  try {
    if (text.size() > 2 && text[0] == '&' &&
        (text[1] == 'h' || text[1] == 'H')) {
      value = stoi(text.substr(2), nullptr, 16);
    } else if (text.size() > 2 && text[0] == '&' &&
               (text[1] == 'o' || text[1] == 'O')) {
      value = stoi(text.substr(2), nullptr, 8);
    } else if (text.size() > 2 && text[0] == '&' &&
               (text[1] == 'b' || text[1] == 'B')) {
      value = stoi(text.substr(2), nullptr, 2);
    } else {
      value = stoi(text);
    }
  } catch (...) {
    return false;
  }

  return true;
}

inline int normalizedTileBankLiteral(shared_ptr<ActionNode> action,
                                     bool& is_literal) {
  int value = 0;
  is_literal = getNumericLiteral(action, value);
  if (!is_literal) return 0;
  return value > 2 ? 3 : value;
}

#endif  // COMPILER_TILE_PARAMETER_H_INCLUDED
