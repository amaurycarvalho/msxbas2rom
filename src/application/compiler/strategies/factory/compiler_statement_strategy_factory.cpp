#include "compiler_statement_strategy_factory.h"

CompilerStatementStrategyFactory::CompilerStatementStrategyFactory() {
  strategies["REM"] = &noopStrategy;
  strategies["'"] = &noopStrategy;

  strategies["CLEAR"] = &basicStrategy;
  strategies["DEF"] = &basicStrategy;
  strategies["DEFINT"] = &basicStrategy;
  strategies["DEFSNG"] = &basicStrategy;
  strategies["DEFDBL"] = &basicStrategy;
  strategies["DEFSTR"] = &basicStrategy;
  strategies["CLS"] = &basicStrategy;
  strategies["BEEP"] = &basicStrategy;
  strategies["LET"] = &basicStrategy;
  strategies["TIME"] = &basicStrategy;
  strategies["DIM"] = &basicStrategy;
  strategies["REDIM"] = &basicStrategy;
  strategies["RANDOMIZE"] = &basicStrategy;

  strategies["END"] = &controlStrategy;
  strategies["GOTO"] = &controlStrategy;
  strategies["GOSUB"] = &controlStrategy;
  strategies["RETURN"] = &controlStrategy;
  strategies["IF"] = &controlStrategy;
  strategies["FOR"] = &controlStrategy;
  strategies["NEXT"] = &controlStrategy;
  strategies["ON"] = &controlStrategy;
  strategies["INTERVAL"] = &controlStrategy;
  strategies["STOP"] = &controlStrategy;

  strategies["PRINT"] = &ioStrategy;
  strategies["INPUT"] = &ioStrategy;
  strategies["SOUND"] = &ioStrategy;
  strategies["OUT"] = &ioStrategy;
  strategies["POKE"] = &ioStrategy;
  strategies["VPOKE"] = &ioStrategy;
  strategies["IPOKE"] = &ioStrategy;
  strategies["DATA"] = &ioStrategy;
  strategies["IDATA"] = &ioStrategy;
  strategies["READ"] = &ioStrategy;
  strategies["IREAD"] = &ioStrategy;
  strategies["RESTORE"] = &ioStrategy;
  strategies["IRESTORE"] = &ioStrategy;
  strategies["RESUME"] = &ioStrategy;
  strategies["WAIT"] = &ioStrategy;
  strategies["SWAP"] = &ioStrategy;
  strategies["CALL"] = &ioStrategy;
  strategies["CMD"] = &ioStrategy;
  strategies["MAXFILES"] = &ioStrategy;
  strategies["OPEN"] = &ioStrategy;
  strategies["OPEN_GRP"] = &ioStrategy;
  strategies["CLOSE"] = &ioStrategy;
  strategies["FILE"] = &ioStrategy;
  strategies["TEXT"] = &ioStrategy;
  strategies["BLOAD"] = &ioStrategy;

  strategies["PLAY"] = &graphicsStrategy;
  strategies["DRAW"] = &graphicsStrategy;
  strategies["LOCATE"] = &graphicsStrategy;
  strategies["SCREEN"] = &graphicsStrategy;
  strategies["WIDTH"] = &graphicsStrategy;
  strategies["COLOR"] = &graphicsStrategy;
  strategies["PSET"] = &graphicsStrategy;
  strategies["PRESET"] = &graphicsStrategy;
  strategies["LINE"] = &graphicsStrategy;
  strategies["PAINT"] = &graphicsStrategy;
  strategies["CIRCLE"] = &graphicsStrategy;
  strategies["COPY"] = &graphicsStrategy;
  strategies["PUT"] = &graphicsStrategy;
  strategies["GET"] = &graphicsStrategy;
  strategies["SET"] = &graphicsStrategy;
  strategies["KEY"] = &graphicsStrategy;
  strategies["STRIG"] = &graphicsStrategy;
  strategies["SPRITE"] = &graphicsStrategy;
}

ICompilerStatementStrategy*
CompilerStatementStrategyFactory::getStrategyByKeyword(const string& keyword) {
  map<string, ICompilerStatementStrategy*>::iterator it =
      strategies.find(keyword);
  if (it == strategies.end()) return 0;

  return it->second;
}
