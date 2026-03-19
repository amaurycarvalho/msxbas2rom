#ifndef EXPRESSION_EVALUATOR_H_INCLUDED
#define EXPRESSION_EVALUATOR_H_INCLUDED

#include <memory>

class ParserContext;
class LexerLineContext;
class Lexeme;

using namespace std;

class ExpressionEvaluator {
 public:
  ExpressionEvaluator(shared_ptr<ParserContext> context);
  ~ExpressionEvaluator();

  /***
   * @brief Expressions syntatic analysis
   * @note Math/Logical/String expressions parser
   * @return True, if syntatic analysis success
   */
  bool evaluate(shared_ptr<LexerLineContext> expression);

  shared_ptr<ParserContext> getContext();
  void setContext(shared_ptr<ParserContext> context);

 private:
  shared_ptr<ParserContext> ctx;

  /***
   * @brief Expressions stack helper (push)
   * @return True, if success
   */
  bool push(shared_ptr<LexerLineContext> expression);

  /***
   * @brief Expressions stack helper (pop)
   */
  void pop(int precedence);

  int getOperatorPrecedence(shared_ptr<Lexeme> lexeme);
  int getOperatorParmCount(shared_ptr<Lexeme> lexeme);
};

#endif  // EXPRESSION_EVALUATOR_H_INCLUDED