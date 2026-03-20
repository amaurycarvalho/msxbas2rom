/***
 * @file lexer_line_evaluator.h
 * @brief MSX-BASIC lexer line evaluator header
 * @author Amaury Carvalho (2019-2026)
 */

#ifndef LEXER_LINE_H_INCLUDED
#define LEXER_LINE_H_INCLUDED

#include "lexer_line_context.h"

/***
 * @class LexerLine
 * @brief It represents a set of MSX-BASIC lexemes forming a line.
 */
class LexerLineEvaluator : public LexerLineContext {
 public:
  bool evaluate();
};

#endif  // LEXER_LINE_H_INCLUDED
