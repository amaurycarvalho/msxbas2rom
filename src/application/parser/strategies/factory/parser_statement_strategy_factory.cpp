/***
 * @file parser_statement_strategy_factory.cpp
 * @brief Factory for parser statement strategies
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#include "parser_statement_strategy_factory.h"

ParserStatementStrategyFactory::ParserStatementStrategyFactory()
    : apostropheStrategy("REM", ParserStatementAction::NoOp, true),
      questionMarkStrategy("PRINT", ParserStatementAction::Print, false),
      underscoreStrategy("CALL", ParserStatementAction::Call, false),
      noOpStrategy(ParserStatementAction::NoOp),
      defStrategy(ParserStatementAction::Def),
      defIntStrategy(ParserStatementAction::DefInt),
      defStrStrategy(ParserStatementAction::DefStr),
      defSngStrategy(ParserStatementAction::DefSng),
      defDblStrategy(ParserStatementAction::DefDbl),
      genericStrategy(ParserStatementAction::Generic),
      screenStrategy(ParserStatementAction::Screen),
      playStrategy(ParserStatementAction::Play),
      letStrategy(ParserStatementAction::Let),
      dimStrategy(ParserStatementAction::Dim),
      printStrategy(ParserStatementAction::Print),
      inputStrategy(ParserStatementAction::Input),
      spriteStrategy(ParserStatementAction::Sprite),
      baseStrategy(ParserStatementAction::Base),
      vdpStrategy(ParserStatementAction::Vdp),
      putStrategy(ParserStatementAction::Put),
      timeStrategy(ParserStatementAction::Time),
      setStrategy(ParserStatementAction::Set),
      getStrategyItem(ParserStatementAction::Get),
      onStrategy(ParserStatementAction::On),
      intervalStrategy(ParserStatementAction::Interval),
      stopStrategy(ParserStatementAction::Stop),
      keyStrategy(ParserStatementAction::Key),
      strigStrategy(ParserStatementAction::Strig),
      colorStrategy(ParserStatementAction::Color),
      dataStrategy(ParserStatementAction::Data),
      idataStrategy(ParserStatementAction::IData),
      ifStrategy(ParserStatementAction::If),
      forStrategy(ParserStatementAction::For),
      nextStrategy(ParserStatementAction::Next),
      psetStrategy(ParserStatementAction::Pset),
      lineStrategy(ParserStatementAction::Line),
      circleStrategy(ParserStatementAction::Circle),
      paintStrategy(ParserStatementAction::Paint),
      copyStrategy(ParserStatementAction::Copy),
      callStrategy(ParserStatementAction::Call),
      cmdStrategy(ParserStatementAction::Cmd),
      openStrategy(ParserStatementAction::Open),
      closeStrategy(ParserStatementAction::Close),
      maxfilesStrategy(ParserStatementAction::Maxfiles) {
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
