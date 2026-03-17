#include "parser_line_evaluator.h"

#include <memory>

#include "lexer_line_evaluator.h"
#include "logger.h"

ParserLineEvaluator::ParserLineEvaluator(
    ParserContext& context, ParserStatementStrategyFactory& strategyFactory,
    ExpressionEvaluator& expressionEvaluator,
    AssignmentEvaluator& assignmentEvaluator)
    : ctx(context),
      statementStrategyFactory(strategyFactory),
      exprEval(expressionEvaluator),
      assignEval(assignmentEvaluator),
      includeLoader(*this) {}

ParserLineEvaluator::~ParserLineEvaluator() = default;

bool ParserLineEvaluator::evaluateLine(LexerLineEvaluator* lexerLine) {
  shared_ptr<Lexeme> lexeme = lexerLine->getFirstLexeme();
  LexerLineEvaluator phrase;
  int if_count = 0;

  ctx.error_line = lexerLine;
  ctx.actionRoot = 0;

  if (lexeme) {
    lexeme = ctx.coalesceSymbols(lexeme);

    if (lexeme->isLiteralNumeric()) {
      auto tag = make_shared<TagNode>();  // register line number tag
      tag->name = lexeme->value;
      tag->value = tag->name;
      tag->lexerLine = lexerLine;
      ctx.tags.push_back(tag);
      ctx.tag = tag;

      while ((lexeme = lexerLine->getNextLexeme())) {
        lexeme = ctx.coalesceSymbols(lexeme);

        if (lexeme->isKeyword("IF")) {
          if_count++;
        }

        if (lexeme->isSeparator(":") && if_count == 0) {
          ctx.actionRoot = 0;
          if (phrase.getLexemeCount())
            if (!evaluatePhrase(&phrase)) return false;
          phrase.clearLexemes();
          continue;

        } else if (lexeme->isOperator("'")) {
          break;

        } else {
          phrase.addLexeme(lexeme);
        }
      }

      if (phrase.getLexemeCount()) {
        ctx.actionRoot = 0;
        if (!evaluatePhrase(&phrase)) return false;
      }

    } else if (lexeme->type == Lexeme::type_keyword) {
      if (lexeme->value == "FILE" || lexeme->value == "TEXT") {
        ctx.resourceCount++;

        auto tag = make_shared<TagNode>();  // register line number tag
        tag->name = "DIRECTIVE";
        tag->value = tag->name;
        tag->lexerLine = lexerLine;
        ctx.tags.push_back(tag);
        ctx.tag = tag;

        auto action = make_shared<ActionNode>(lexeme);
        ctx.pushActionRoot(action);

        lexeme = lexerLine->getNextLexeme();
        if (lexeme) ctx.pushActionFromLexeme(lexeme);

        ctx.popActionRoot();

      } else if (lexeme->value == "INCLUDE") {
        if ((lexeme = lexerLine->getNextLexeme())) {
          if (lexeme->type == Lexeme::type_literal &&
              lexeme->subtype == Lexeme::subtype_string) {
            if (!includeLoader.load(lexeme, lexerLine)) {
              ctx.logger->error(
                  "INCLUDE file not found or with content syntax error");
              return false;
            }

          } else {
            ctx.logger->error("Invalid parameter in INCLUDE keyword");
            return false;
          }
        }
      }
    }
  }

  return true;
}

bool ParserLineEvaluator::evaluatePhrase(LexerLineContext* phrase) {
  shared_ptr<Lexeme> lexeme = phrase->getFirstLexeme();

  if (lexeme) {
    lexeme = ctx.coalesceSymbols(lexeme);

    if (lexeme->type == Lexeme::type_identifier ||
        (lexeme->type == Lexeme::type_keyword &&
         lexeme->subtype == Lexeme::subtype_function &&
         lexeme->value != "STRIG")) {
      phrase->setLexemeBOF();
      return assignEval.evaluate(phrase);
    } else if (lexeme->type == Lexeme::type_keyword) {
      return evaluateStatement(phrase);
    } else if (lexeme->isOperator("'")) {
      return evaluateStatement(phrase);
    } else if (lexeme->isOperator("_")) {
      return evaluateStatement(phrase);
    }
  }

  ctx.logger->error("Invalid keyword/identifier");
  return false;
}

bool ParserLineEvaluator::evaluateStatement(LexerLineContext* statement) {
  shared_ptr<Lexeme> lexeme;
  IParserStatementStrategy* strategy;
  shared_ptr<ActionNode> actionSaved = ctx.actionRoot;
  unsigned int actionCount = ctx.actionStack.size();
  bool result = true;

  lexeme = statement->getFirstLexeme();

  if (lexeme) {
    lexeme = ctx.coalesceSymbols(lexeme);

    auto action = make_shared<ActionNode>(lexeme);
    ctx.pushActionRoot(action);

    strategy = statementStrategyFactory.getStrategyByKeyword(lexeme->value);
    if (strategy) {
      result = strategy->execute(ctx, statement, lexeme);
      if (lexeme->value == "IF") return result;
    } else {
      ctx.logger->error("Invalid keyword / identifier");
      result = false;
    }

    ctx.popActionRoot();
  }

  while (ctx.actionStack.size() > actionCount) ctx.actionStack.pop();

  ctx.actionRoot = actionSaved;

  return result;
}
