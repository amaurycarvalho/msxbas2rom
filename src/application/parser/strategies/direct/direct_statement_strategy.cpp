#include "direct_statement_strategy.h"

#include "parser.h"

bool DirectStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                      Lexeme* lexeme) {
  (void)lexeme;

  switch (type) {
    case SCREEN:
      return parser.evalCmdScreen(statement);
    case SPRITE:
      return parser.evalCmdSprite(statement);
    case BASE:
      return parser.evalCmdBase(statement);
    case VDP:
      return parser.evalCmdVdp(statement);
    case PUT:
      return parser.evalCmdPut(statement);
    case TIME:
      return parser.evalCmdTime(statement);
    case SET:
      return parser.evalCmdSet(statement);
    case GET:
      return parser.evalCmdGet(statement);
    default:
      return false;
  }
}
