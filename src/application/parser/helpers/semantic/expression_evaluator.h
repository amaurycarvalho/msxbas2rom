#ifndef EXPRESSION_EVALUATOR_H_INCLUDED
#define EXPRESSION_EVALUATOR_H_INCLUDED

#include "parser_context.h"

class ExpressionEvaluator {
 public:
  ExpressionEvaluator(ParserContext& context);

  /***
   * @brief Expressions syntatic analysis
   * @note Math/Logical/String expressions parser
   * @return True, if syntatic analysis success
   */
  bool evaluate(LexerLine* expression);

 private:
  ParserContext& ctx;

  /***
   * @brief Expressions stack helper (push)
   * @return True, if success
   */
  bool push(LexerLine* expression);

  /***
   * @brief Expressions stack helper (pop)
   */
  void pop(int precedence);

  int getOperatorPrecedence(Lexeme* lexeme);
  int getOperatorParmCount(Lexeme* lexeme);
};

#endif  // EXPRESSION_EVALUATOR_H_INCLUDED