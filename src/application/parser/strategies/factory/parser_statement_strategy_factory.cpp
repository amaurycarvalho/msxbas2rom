#include "parser_statement_strategy_factory.h"

ParserStatementStrategyFactory::ParserStatementStrategyFactory()
    : screenStrategy(DirectStatementStrategy::SCREEN),
      spriteStrategy(DirectStatementStrategy::SPRITE),
      baseStrategy(DirectStatementStrategy::BASE),
      vdpStrategy(DirectStatementStrategy::VDP),
      putStrategy(DirectStatementStrategy::PUT),
      timeStrategy(DirectStatementStrategy::TIME),
      setStrategy(DirectStatementStrategy::SET),
      getStatementStrategy(DirectStatementStrategy::GET) {
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
}

IParserStatementStrategy* ParserStatementStrategyFactory::getStrategyByKeyword(
    const string& keyword) {
  map<string, IParserStatementStrategy*>::iterator it = strategies.find(keyword);
  if (it == strategies.end()) return 0;

  return it->second;
}
