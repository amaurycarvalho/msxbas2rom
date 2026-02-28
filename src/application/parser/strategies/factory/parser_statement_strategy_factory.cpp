/***
 * @file parser_statement_strategy_factory.cpp
 * @brief Factory for parser statement strategies
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#include "parser_statement_strategy_factory.h"

#include "parser.h"

ParserStatementStrategyFactory::ParserStatementStrategyFactory()
    : apostropheStrategy("REM", true, 0),
      questionMarkStrategy("PRINT", false, &Parser::eval_cmd_print),
      underscoreStrategy("CALL", false, &Parser::eval_cmd_call),
      noOpStrategy(0),
      defStrategy(0),
      defIntStrategy(2),
      defStrStrategy(3),
      defSngStrategy(4),
      defDblStrategy(8),
      screenStrategy(&Parser::eval_cmd_screen),
      letStrategy(&Parser::eval_cmd_let),
      dimStrategy(&Parser::eval_cmd_dim),
      printStrategy(&Parser::eval_cmd_print),
      spriteStrategy(&Parser::eval_cmd_sprite),
      baseStrategy(&Parser::eval_cmd_base),
      vdpStrategy(&Parser::eval_cmd_vdp),
      putStrategy(&Parser::eval_cmd_put),
      timeStrategy(&Parser::eval_cmd_time),
      setStrategy(&Parser::eval_cmd_set),
      getStrategyItem(&Parser::eval_cmd_get),
      onStrategy(&Parser::eval_cmd_on),
      intervalStrategy(&Parser::eval_cmd_interval),
      stopStrategy(&Parser::eval_cmd_stop),
      keyStrategy(&Parser::eval_cmd_key),
      strigStrategy(&Parser::eval_cmd_strig),
      colorStrategy(&Parser::eval_cmd_color),
      dataStrategy(Lexeme::subtype_string),
      idataStrategy(Lexeme::subtype_integer_data),
      forStrategy(&Parser::eval_cmd_for),
      nextStrategy(&Parser::eval_cmd_next),
      psetStrategy(&Parser::eval_cmd_pset),
      lineStrategy(&Parser::eval_cmd_line),
      circleStrategy(&Parser::eval_cmd_circle),
      paintStrategy(&Parser::eval_cmd_paint),
      copyStrategy(&Parser::eval_cmd_copy),
      callStrategy(&Parser::eval_cmd_call),
      cmdStrategy(&Parser::eval_cmd_cmd),
      openStrategy(&Parser::eval_cmd_open),
      closeStrategy(&Parser::eval_cmd_close),
      maxfilesStrategy(&Parser::eval_cmd_maxfiles) {
  strategies["REM"] = &noOpStrategy;
  strategies["CLS"] = &noOpStrategy;
  strategies["END"] = &noOpStrategy;
  strategies["BEEP"] = &noOpStrategy;
  strategies["RANDOMIZE"] = &noOpStrategy;

  strategies["'"] = &apostropheStrategy;
  strategies["?"] = &questionMarkStrategy;
  strategies["_"] = &underscoreStrategy;

  strategies["DEF"] = &defStrategy;
  strategies["DEFINT"] = &defIntStrategy;
  strategies["DEFSTR"] = &defStrStrategy;
  strategies["DEFSNG"] = &defSngStrategy;
  strategies["DEFDBL"] = &defDblStrategy;

  strategies["WIDTH"] = &genericStrategy;
  strategies["CLEAR"] = &genericStrategy;
  strategies["ERASE"] = &genericStrategy;
  strategies["LOCATE"] = &genericStrategy;
  strategies["DRAW"] = &genericStrategy;
  strategies["GOTO"] = &genericStrategy;
  strategies["GOSUB"] = &genericStrategy;
  strategies["RETURN"] = &genericStrategy;
  strategies["SOUND"] = &genericStrategy;
  strategies["RESTORE"] = &genericStrategy;
  strategies["RESUME"] = &genericStrategy;
  strategies["READ"] = &genericStrategy;
  strategies["IREAD"] = &genericStrategy;
  strategies["IRESTORE"] = &genericStrategy;
  strategies["POKE"] = &genericStrategy;
  strategies["IPOKE"] = &genericStrategy;
  strategies["VPOKE"] = &genericStrategy;
  strategies["OUT"] = &genericStrategy;
  strategies["SWAP"] = &genericStrategy;
  strategies["WAIT"] = &genericStrategy;
  strategies["SEED"] = &genericStrategy;
  strategies["BLOAD"] = &genericStrategy;

  strategies["SCREEN"] = &screenStrategy;
  strategies["PLAY"] = &playStrategy;
  strategies["LET"] = &letStrategy;
  strategies["DIM"] = &dimStrategy;
  strategies["REDIM"] = &dimStrategy;
  strategies["PRINT"] = &printStrategy;
  strategies["INPUT"] = &inputStrategy;
  strategies["SPRITE"] = &spriteStrategy;
  strategies["BASE"] = &baseStrategy;
  strategies["VDP"] = &vdpStrategy;
  strategies["PUT"] = &putStrategy;
  strategies["TIME"] = &timeStrategy;
  strategies["SET"] = &setStrategy;
  strategies["GET"] = &getStrategyItem;
  strategies["ON"] = &onStrategy;
  strategies["INTERVAL"] = &intervalStrategy;
  strategies["STOP"] = &stopStrategy;
  strategies["KEY"] = &keyStrategy;
  strategies["STRIG"] = &strigStrategy;
  strategies["COLOR"] = &colorStrategy;
  strategies["DATA"] = &dataStrategy;
  strategies["IDATA"] = &idataStrategy;
  strategies["IF"] = &ifStrategy;
  strategies["FOR"] = &forStrategy;
  strategies["NEXT"] = &nextStrategy;
  strategies["PSET"] = &psetStrategy;
  strategies["PRESET"] = &psetStrategy;
  strategies["LINE"] = &lineStrategy;
  strategies["CIRCLE"] = &circleStrategy;
  strategies["PAINT"] = &paintStrategy;
  strategies["COPY"] = &copyStrategy;
  strategies["CALL"] = &callStrategy;
  strategies["CMD"] = &cmdStrategy;
  strategies["OPEN"] = &openStrategy;
  strategies["CLOSE"] = &closeStrategy;
  strategies["MAX"] = &maxfilesStrategy;
}

IParserStatementStrategy* ParserStatementStrategyFactory::getStrategy(
    const string& keyword) {
  unordered_map<string, IParserStatementStrategy*>::iterator it =
      strategies.find(keyword);
  if (it != strategies.end()) return it->second;
  return 0;
}
