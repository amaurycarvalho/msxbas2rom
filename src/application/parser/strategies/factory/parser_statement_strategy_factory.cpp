#include "parser_statement_strategy_factory.h"

ParserStatementStrategyFactory::ParserStatementStrategyFactory()
    : screenStrategy(DirectStatementStrategy::SCREEN),
      spriteStrategy(DirectStatementStrategy::SPRITE),
      baseStrategy(DirectStatementStrategy::BASE),
      vdpStrategy(DirectStatementStrategy::VDP),
      putStrategy(DirectStatementStrategy::PUT),
      timeStrategy(DirectStatementStrategy::TIME),
      setStrategy(DirectStatementStrategy::SET),
      getStatementStrategy(DirectStatementStrategy::GET),
      onStrategy(DirectStatementStrategy::ON),
      intervalStrategy(DirectStatementStrategy::INTERVAL),
      stopStrategy(DirectStatementStrategy::STOP),
      keyStrategy(DirectStatementStrategy::KEY),
      strigStrategy(DirectStatementStrategy::STRIG),
      colorStrategy(DirectStatementStrategy::COLOR),
      cmdStrategy(DirectStatementStrategy::CMD),
      openStrategy(DirectStatementStrategy::OPEN),
      closeStrategy(DirectStatementStrategy::CLOSE),
      maxStrategy(DirectStatementStrategy::MAX),
      ifStrategy(DirectStatementStrategy::IF),
      forStrategy(DirectStatementStrategy::FOR),
      nextStrategy(DirectStatementStrategy::NEXT),
      psetStrategy(DirectStatementStrategy::PSET),
      lineStrategy(DirectStatementStrategy::LINE),
      circleStrategy(DirectStatementStrategy::CIRCLE),
      paintStrategy(DirectStatementStrategy::PAINT),
      copyStrategy(DirectStatementStrategy::COPY) {
  strategies["REM"] = &noopStrategy;
  strategies["CLS"] = &noopStrategy;
  strategies["END"] = &noopStrategy;
  strategies["BEEP"] = &noopStrategy;
  strategies["RANDOMIZE"] = &noopStrategy;
  strategies["'"] = &noopStrategy;

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
  strategies["PLAY"] = &genericStrategy;

  strategies["LET"] = &letStrategy;
  strategies["DIM"] = &dimStrategy;
  strategies["REDIM"] = &dimStrategy;
  strategies["PRINT"] = &printStrategy;
  strategies["?"] = &printStrategy;
  strategies["INPUT"] = &inputStrategy;
  strategies["DATA"] = &dataStrategy;
  strategies["IDATA"] = &idataStrategy;
  strategies["SCREEN"] = &screenStrategy;
  strategies["SPRITE"] = &spriteStrategy;
  strategies["BASE"] = &baseStrategy;
  strategies["VDP"] = &vdpStrategy;
  strategies["PUT"] = &putStrategy;
  strategies["TIME"] = &timeStrategy;
  strategies["SET"] = &setStrategy;
  strategies["GET"] = &getStatementStrategy;
  strategies["ON"] = &onStrategy;
  strategies["INTERVAL"] = &intervalStrategy;
  strategies["STOP"] = &stopStrategy;
  strategies["KEY"] = &keyStrategy;
  strategies["STRIG"] = &strigStrategy;
  strategies["COLOR"] = &colorStrategy;
  strategies["CMD"] = &cmdStrategy;
  strategies["OPEN"] = &openStrategy;
  strategies["CLOSE"] = &closeStrategy;
  strategies["MAX"] = &maxStrategy;
  strategies["CALL"] = &callStrategy;
  strategies["_"] = &callStrategy;
  strategies["DEF"] = &defStrategy;
  strategies["DEFINT"] = &defStrategy;
  strategies["DEFSTR"] = &defStrategy;
  strategies["DEFSNG"] = &defStrategy;
  strategies["DEFDBL"] = &defStrategy;
  strategies["IF"] = &ifStrategy;
  strategies["FOR"] = &forStrategy;
  strategies["NEXT"] = &nextStrategy;
  strategies["PSET"] = &psetStrategy;
  strategies["PRESET"] = &psetStrategy;
  strategies["LINE"] = &lineStrategy;
  strategies["CIRCLE"] = &circleStrategy;
  strategies["PAINT"] = &paintStrategy;
  strategies["COPY"] = &copyStrategy;
}

IParserStatementStrategy* ParserStatementStrategyFactory::getStrategyByKeyword(
    const string& keyword) {
  map<string, IParserStatementStrategy*>::iterator it = strategies.find(keyword);
  if (it == strategies.end()) return 0;

  return it->second;
}
