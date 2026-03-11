#include "parser_line_evaluator.h"

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

bool ParserLineEvaluator::evaluateLine(LexerLine* lexerLine) {
  Lexeme* lexeme = lexerLine->getFirstLexeme();
  ActionNode* action;
  LexerLine phrase;
  int if_count = 0;

  ctx.error_line = lexerLine;
  ctx.actionRoot = 0;

  if (lexeme) {
    lexeme = ctx.coalesceSymbols(lexeme);

    if (lexeme->isLiteralNumeric()) {
      ctx.tag = new TagNode();  // register line number tag
      ctx.tag->name = lexeme->value;
      ctx.tag->value = ctx.tag->name;
      ctx.tags.push_back(ctx.tag);

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

        ctx.tag = new TagNode();  // register line number tag
        ctx.tag->name = "DIRECTIVE";
        ctx.tag->value = ctx.tag->name;
        ctx.tags.push_back(ctx.tag);

        action = new ActionNode(lexeme);
        ctx.pushActionRoot(action);

        lexeme = lexerLine->getNextLexeme();
        if (lexeme) ctx.pushActionFromLexeme(lexeme);

        ctx.popActionRoot();

      } else if (lexeme->value == "INCLUDE") {
        if ((lexeme = lexerLine->getNextLexeme())) {
          if (lexeme->type == Lexeme::type_literal &&
              lexeme->subtype == Lexeme::subtype_string) {
            if (!includeLoader.load(lexeme)) {
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

bool ParserLineEvaluator::evaluatePhrase(LexerLine* phrase) {
  Lexeme* lexeme = phrase->getFirstLexeme();

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

bool ParserLineEvaluator::evaluateStatement(LexerLine* statement) {
  Lexeme* lexeme;
  ActionNode* action;
  IParserStatementStrategy* strategy;
  ActionNode* actionSaved = ctx.actionRoot;
  unsigned int actionCount = ctx.actionStack.size();
  bool result = true;

  lexeme = statement->getFirstLexeme();

  if (lexeme) {
    lexeme = ctx.coalesceSymbols(lexeme);

    action = new ActionNode(lexeme);
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
