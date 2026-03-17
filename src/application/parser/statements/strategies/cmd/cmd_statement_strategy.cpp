#include "cmd_statement_strategy.h"

#include "generic_statement_strategy.h"

bool CmdStatementStrategy::parseStatement(ParserContext& context,
                                          LexerLineContext* statement) {
  ActionNode* action;
  shared_ptr<Lexeme> lexeme = statement->getNextLexeme();

  if (!lexeme) return false;

  action = new ActionNode(lexeme);
  context.pushActionRoot(action);

  if (lexeme->value == "WRTFNT" || lexeme->value == "SETFNT" ||
      lexeme->value == "WRTCHR")
    context.has_font = true;

  if (lexeme->value == "PT3LOAD" || lexeme->value == "PT3PLAY")
    context.has_pt3 = true;

  if (lexeme->value == "PLYLOAD" || lexeme->value == "PLYPLAY" ||
      lexeme->value == "PLYSONG")
    context.has_akm = true;

  if (lexeme->value == "MTF") context.has_mtf = true;

  if (lexeme->value == "RESTORE") context.has_resource_restore = true;

  GenericStatementStrategy genericStrategy;
  bool result = genericStrategy.parseStatement(context, statement);

  if (result) {
    context.popActionRoot();
  }

  return result;
}

bool CmdStatementStrategy::execute(ParserContext& context,
                                   LexerLineContext* statement,
                                   shared_ptr<Lexeme> lexeme) {
  (void)lexeme;
  return parseStatement(context, statement);
}
