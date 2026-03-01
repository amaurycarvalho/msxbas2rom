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
    case ON:
      return parser.evalCmdOn(statement);
    case INTERVAL:
      return parser.evalCmdInterval(statement);
    case STOP:
      return parser.evalCmdStop(statement);
    case KEY:
      return parser.evalCmdKey(statement);
    case STRIG:
      return parser.evalCmdStrig(statement);
    case COLOR:
      return parser.evalCmdColor(statement);
    case CMD:
      return parser.evalCmdCmd(statement);
    case OPEN:
      return parser.evalCmdOpen(statement);
    case CLOSE:
      return parser.evalCmdClose(statement);
    case MAX:
      return parser.evalCmdMaxfiles(statement);
    default:
      return false;
  }
}
