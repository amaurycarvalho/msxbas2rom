#include "cmd_statement_strategy.h"

#include "generic_statement_strategy.h"
#include "parser.h"

bool CmdStatementStrategy::parseStatement(Parser& parser, LexerLine* statement) {
  ActionNode* action;
  Lexeme* lexeme = statement->getNextLexeme();

  if (!lexeme) return false;

  action = new ActionNode(lexeme);
  parser.pushActionNodeRoot(action);

  ParserContext& ctx = parser.getContext();

  if (lexeme->value == "WRTFNT" || lexeme->value == "SETFNT" ||
      lexeme->value == "WRTCHR")
    ctx.has_font = true;

  if (lexeme->value == "PT3LOAD" || lexeme->value == "PT3PLAY") ctx.has_pt3 = true;

  if (lexeme->value == "PLYLOAD" || lexeme->value == "PLYPLAY" ||
      lexeme->value == "PLYSONG")
    ctx.has_akm = true;

  if (lexeme->value == "MTF") ctx.has_mtf = true;

  if (lexeme->value == "RESTORE") ctx.has_resource_restore = true;

  GenericStatementStrategy genericStrategy;
  bool result = genericStrategy.parseStatement(parser, statement);

  if (result) {
    parser.popActionNodeRoot();
  }

  return result;
}

bool CmdStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                   Lexeme* lexeme) {
  (void)lexeme;
  return parseStatement(parser, statement);
}
