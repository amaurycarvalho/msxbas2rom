#include "cmd_statement_strategy.h"

#include "action_node.h"
#include "generic_statement_strategy.h"
#include "lexeme.h"
#include "lexer_line_context.h"

bool CmdStatementStrategy::parseStatement(
    shared_ptr<ParserContext> context, shared_ptr<LexerLineContext> statement) {
  shared_ptr<ActionNode> action;
  shared_ptr<Lexeme> lexeme = statement->getNextLexeme();

  if (!lexeme) return false;

  action = make_shared<ActionNode>(lexeme);
  context->pushActionRoot(action);

  if (lexeme->value == "WRTFNT" || lexeme->value == "SETFNT" ||
      lexeme->value == "WRTCHR")
    context->has_font = true;

  if (lexeme->value == "PT3LOAD" || lexeme->value == "PT3PLAY")
    context->has_pt3 = true;

  if (lexeme->value == "PLYLOAD" || lexeme->value == "PLYPLAY" ||
      lexeme->value == "PLYSONG")
    context->has_akm = true;

  if (lexeme->value == "MTF") context->has_mtf = true;

  if (lexeme->value == "RESTORE") context->has_resource_restore = true;

  GenericStatementStrategy genericStrategy;
  bool result = genericStrategy.parseStatement(context, statement);

  if (result) {
    context->popActionRoot();
  }

  return result;
}

bool CmdStatementStrategy::execute(shared_ptr<ParserContext> context,
                                   shared_ptr<LexerLineContext> statement,
                                   shared_ptr<Lexeme> lexeme) {
  (void)lexeme;
  return parseStatement(context, statement);
}
