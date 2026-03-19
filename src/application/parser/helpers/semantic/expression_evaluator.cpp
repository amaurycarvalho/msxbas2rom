/***
 * @file expression_evaluator.cpp
 * @brief Expressions syntatic analysis class code
 * @author Amaury Carvalho (2019-2026)
 */

#include "expression_evaluator.h"

#include <stack>

#include "action_node.h"
#include "expression_evaluator.h"
#include "lexeme.h"
#include "lexer_line_evaluator.h"
#include "logger.h"
#include "parser_context.h"

ExpressionEvaluator::ExpressionEvaluator(shared_ptr<ParserContext> context)
    : ctx(context) {}

ExpressionEvaluator::~ExpressionEvaluator() = default;

bool ExpressionEvaluator::evaluate(shared_ptr<LexerLineContext> expression) {
  shared_ptr<ActionNode> actionSaved = ctx->actionRoot;
  unsigned int actionCount = ctx->actionStack.size();

  while (!ctx->expressionList.empty()) ctx->expressionList.pop();

  if (!push(expression)) return false;

  while (!ctx->expressionList.empty()) {
    pop(1);
  }

  while (ctx->actionStack.size() > actionCount) ctx->actionStack.pop();

  ctx->actionRoot = actionSaved;

  return true;
}

shared_ptr<ParserContext> ExpressionEvaluator::getContext() {
  return ctx;
}

void ExpressionEvaluator::setContext(shared_ptr<ParserContext> context) {
  this->ctx = context;
}

bool ExpressionEvaluator::push(shared_ptr<LexerLineContext> expression) {
  stack<shared_ptr<Lexeme>> operatorStack;
  shared_ptr<Lexeme> lexeme, next_lexeme, check_lexeme;
  shared_ptr<LexerLineContext> functionLexemes =
      make_shared<LexerLineContext>();
  int thisPreced, stackPreced;
  int outputCount = 0, sepcount = 0, parmcount = 0;
  bool ok, unary = false, lastWasFunction = false, lastWasIdentifier = false;

  lexeme = expression->getNextLexeme();
  if (lexeme) {
    lexeme = ctx->coalesceSymbols(lexeme);
    lastWasIdentifier = (lexeme->type == Lexeme::type_identifier);
  }

  while (lexeme) {
    lexeme = ctx->coalesceSymbols(lexeme);

    if (lexeme->type == Lexeme::type_keyword &&
        lexeme->subtype == Lexeme::subtype_function && lexeme->value == "USR") {
      check_lexeme = expression->getNextLexeme();
      if (check_lexeme) {
        if (check_lexeme->isLiteralNumeric()) {
          lexeme->value += check_lexeme->value;
          lexeme->name = lexeme->value;
        } else
          expression->getPreviousLexeme();
      } else
        expression->getPreviousLexeme();
    }

    // if operators, push to operators stack

    if (lexeme->type == Lexeme::type_operator ||
        lexeme->type == Lexeme::type_separator) {
      // test when first operand in expression or unary operator

      if (outputCount == 0 || unary) {
        if (outputCount == 0 && lexeme->value == ")") {
          ctx->eval_expr_error = true;
          ctx->logger->error("Mismatched parentheses error");
          return false;
        } else if (lexeme->value == "=") {
          next_lexeme = operatorStack.top();
          if (next_lexeme->value == "<") {
            next_lexeme->value = "<=";
            lexeme = expression->getNextLexeme();
            lexeme = ctx->coalesceSymbols(lexeme);
            continue;
          } else if (next_lexeme->value == ">") {
            next_lexeme->value = ">=";
            lexeme = expression->getNextLexeme();
            lexeme = ctx->coalesceSymbols(lexeme);
            continue;
          } else {
            ctx->eval_expr_error = true;
            ctx->logger->error("Invalid = symbol in expression");
            return false;
          }
        } else if (lexeme->value == ">") {
          next_lexeme = operatorStack.top();
          if (next_lexeme->value == "<") {
            next_lexeme->value = "<>";
            lexeme = expression->getNextLexeme();
            lexeme = ctx->coalesceSymbols(lexeme);
            continue;
          } else {
            ctx->eval_expr_error = true;
            ctx->logger->error("Invalid > symbol in expression");
            return false;
          }
        } else if (lexeme->value != "+" && lexeme->value != "-" &&
                   lexeme->value != "(" && lexeme->value != ")" &&
                   lexeme->value != "NOT") {
          ctx->eval_expr_error = true;
          ctx->logger->error("Invalid expression unary symbol");
          return false;
        } else {
          if (lexeme->value == "+" ||
              lexeme->value == "-") {  // unary plus or minus
            lexeme->isUnary = true;
            operatorStack.push(lexeme);
            lexeme = expression->getNextLexeme();
            lexeme = ctx->coalesceSymbols(lexeme);
            unary = false;
            continue;
          }
        }
      }

      if (lexeme->value == "(") {
        if (lastWasFunction || lastWasIdentifier) {
          if (ctx->expressionList.empty()) {
            ctx->eval_expr_error = true;
            ctx->logger->error(
                "Invalid FUNCTION or ARRAY declaration in expression");
            return false;
          }

          next_lexeme = ctx->expressionList.top();
          ctx->expressionList.pop();

          // parse function/array parameters
          functionLexemes->clearLexemes();
          ok = false;
          sepcount = 0;
          parmcount = 0;
          while ((lexeme = expression->getNextLexeme())) {
            lexeme = ctx->coalesceSymbols(lexeme);
            if (lexeme->value == "(") {
              sepcount++;
            } else if (lexeme->value == ")") {
              if (sepcount)
                sepcount--;
              else {
                ok = true;
                functionLexemes->setLexemeBOF();
                if (!push(functionLexemes)) return false;
                functionLexemes->clearLexemes();
                parmcount++;
                break;
              }
            } else if (lexeme->value == "," && sepcount == 0) {
              functionLexemes->setLexemeBOF();
              if (!push(functionLexemes)) return false;
              functionLexemes->clearLexemes();
              parmcount++;
              continue;
            }
            functionLexemes->addLexeme(lexeme);
          }
          if (!ok) {
            ctx->eval_expr_error = true;
            ctx->logger->error(
                "Mismatched parentheses error in function or array");
            return false;
          }

          if (!parmcount) {
            ctx->eval_expr_error = true;
            ctx->logger->error(
                "Invalid FUNCTION or ARRAY declaration in expression (missing "
                "parameters?)");
            return false;
          }

          next_lexeme->parm_count = parmcount;

          ctx->pushStackFromLexeme(next_lexeme);

          lastWasFunction = false;
          lastWasIdentifier = false;
          unary = false;

        } else {
          operatorStack.push(lexeme);
          unary = true;
        }

        lexeme = expression->getNextLexeme();
        lexeme = ctx->coalesceSymbols(lexeme);
        continue;
      }

      lastWasIdentifier = false;
      lastWasFunction = false;

      // test this operator precedence against operators in stack

      thisPreced = getOperatorPrecedence(lexeme);

      while (!operatorStack.empty()) {
        next_lexeme = operatorStack.top();
        stackPreced = getOperatorPrecedence(next_lexeme);
        if ((thisPreced <= stackPreced ||
             next_lexeme->type == Lexeme::type_keyword) &&
            next_lexeme->value != "(") {
          ctx->pushStackFromLexeme(next_lexeme);
          operatorStack.pop();
        } else
          break;
      }

      unary = true;

      if (lexeme->value == ")") {
        unary = false;
        ok = false;
        while (!operatorStack.empty()) {
          next_lexeme = operatorStack.top();
          if (next_lexeme->value == "(") {
            operatorStack.pop();
            ok = true;
            break;
          } else {
            ctx->pushStackFromLexeme(next_lexeme);
            operatorStack.pop();
          }
        }
        if (!ok) {
          ctx->eval_expr_error = true;
          ctx->logger->error("Mismatched parentheses error");
          return false;  // parentheses is missing
        }
      } else
        operatorStack.push(lexeme);

    } else {
      // else, push operand to output stack

      lastWasIdentifier = (lexeme->type == Lexeme::type_identifier);
      lastWasFunction = (lexeme->type == Lexeme::type_keyword);

      ctx->pushStackFromLexeme(lexeme);

      outputCount++;
      unary = false;
    }

    lexeme = expression->getNextLexeme();
    lexeme = ctx->coalesceSymbols(lexeme);
  }

  // if operators stack is not empty, copy to output stack
  while (!operatorStack.empty()) {
    next_lexeme = operatorStack.top();
    ctx->pushStackFromLexeme(next_lexeme);
    operatorStack.pop();
  }

  return true;
}

void ExpressionEvaluator::pop(int precedence) {
  shared_ptr<Lexeme> lexeme;
  int k;

  while (precedence && !ctx->expressionList.empty()) {
    lexeme = ctx->expressionList.top();
    ctx->expressionList.pop();

    ctx->pushActionFromLexeme(lexeme);

    k = getOperatorParmCount(lexeme);
    if (k) {
      pop(k);
      ctx->popActionRoot();
    }

    precedence--;
  }
}

int ExpressionEvaluator::getOperatorPrecedence(shared_ptr<Lexeme> lexeme) {
  int result;

  if (lexeme->value == "IMP")
    result = 1;
  else if (lexeme->value == "EQV")
    result = 2;
  else if (lexeme->value == "XOR")
    result = 3;
  else if (lexeme->value == "OR")
    result = 4;
  else if (lexeme->value == "AND")
    result = 5;
  else if (lexeme->value == "NOT")
    result = 6;
  else if (lexeme->value == ">" || lexeme->value == "<" ||
           lexeme->value == "=" || lexeme->value == ">=" ||
           lexeme->value == "<=" || lexeme->value == "<>")
    result = 7;
  else if (lexeme->value == "+" || lexeme->value == "-")
    result = 8;
  else if (lexeme->value == "*" || lexeme->value == "/" ||
           lexeme->value == "\\" || lexeme->value == "MOD" ||
           lexeme->value == "SHR" || lexeme->value == "SHL")
    result = 9;
  else if (lexeme->value == "^")
    result = 10;
  else
    result = 0;

  return result;
}

int ExpressionEvaluator::getOperatorParmCount(shared_ptr<Lexeme> lexeme) {
  int result = 0;

  if (lexeme) {
    if (lexeme->type == Lexeme::type_operator) {
      if (lexeme->value == "NOT" || lexeme->isUnary)
        result = 1;
      else if (lexeme->value == "AND" || lexeme->value == "OR" ||
               lexeme->value == "XOR" || lexeme->value == "EQV" ||
               lexeme->value == "IMP" || lexeme->value == ">" ||
               lexeme->value == "<" || lexeme->value == "=" ||
               lexeme->value == ">=" || lexeme->value == "<=" ||
               lexeme->value == "<>" || lexeme->value == "+" ||
               lexeme->value == "-" || lexeme->value == "*" ||
               lexeme->value == "/" || lexeme->value == "\\" ||
               lexeme->value == "^" || lexeme->value == "MOD" ||
               lexeme->value == "SHR" || lexeme->value == "SHL")
        result = 2;
    } else if (lexeme->type == Lexeme::type_identifier) {
      result = lexeme->parm_count;
    } else if (lexeme->type == Lexeme::type_keyword) {
      if (lexeme->value == "NOT")
        result = 1;
      else if (lexeme->value == "AND" || lexeme->value == "OR" ||
               lexeme->value == "XOR" || lexeme->value == "EQV" ||
               lexeme->value == "IMP" || lexeme->value == "MOD" ||
               lexeme->value == "SHR" || lexeme->value == "SHL")
        result = 2;
      else
        result = lexeme->parm_count;
    }
  }

  return result;
}
