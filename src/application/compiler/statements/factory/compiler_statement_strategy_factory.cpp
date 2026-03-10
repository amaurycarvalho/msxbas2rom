/***
 * @file compiler_statement_strategy_factory.cpp
 * @brief Compiler statement strategy factory
 */

#include "compiler_statement_strategy_factory.h"

CompilerStatementStrategyFactory::CompilerStatementStrategyFactory() {
  strategies["REM"] = &noopStrategy;
  strategies["'"] = &noopStrategy;

  strategies["FILE"] = &fileStrategy;
  strategies["TEXT"] = &textStrategy;

  strategies["CLEAR"] = &clearStrategy;
  strategies["DEF"] = &defStrategy;
  strategies["DEFINT"] = &defintStrategy;
  strategies["DEFSNG"] = &defsngStrategy;
  strategies["DEFDBL"] = &defdblStrategy;
  strategies["DEFSTR"] = &defstrStrategy;
  strategies["DEFUSR"] = &defusrStrategy;
  strategies["CLS"] = &clsStrategy;
  strategies["BEEP"] = &beepStrategy;
  strategies["LET"] = &letStrategy;
  strategies["TIME"] = &timeStrategy;
  strategies["DIM"] = &dimStrategy;
  strategies["REDIM"] = &redimStrategy;
  strategies["RANDOMIZE"] = &randomizeStrategy;

  strategies["END"] = &endStrategy;
  strategies["GOTO"] = &gotoStrategy;
  strategies["GOSUB"] = &gosubStrategy;
  strategies["RETURN"] = &returnStrategy;
  strategies["IF"] = &ifStrategy;
  strategies["FOR"] = &forStrategy;
  strategies["NEXT"] = &nextStrategy;
  strategies["ON"] = &onStrategy;
  strategies["INTERVAL"] = &intervalStrategy;
  strategies["STOP"] = &stopStrategy;

  strategies["PRINT"] = &printStrategy;
  strategies["INPUT"] = &inputStrategy;
  strategies["SOUND"] = &soundStrategy;
  strategies["OUT"] = &outStrategy;
  strategies["POKE"] = &pokeStrategy;
  strategies["VPOKE"] = &vpokeStrategy;
  strategies["IPOKE"] = &ipokeStrategy;
  strategies["DATA"] = &dataStrategy;
  strategies["IDATA"] = &idataStrategy;
  strategies["READ"] = &readStrategy;
  strategies["IREAD"] = &ireadStrategy;
  strategies["RESTORE"] = &restoreStrategy;
  strategies["IRESTORE"] = &irestoreStrategy;
  strategies["RESUME"] = &resumeStrategy;
  strategies["WAIT"] = &waitStrategy;
  strategies["SWAP"] = &swapStrategy;
  strategies["CALL"] = &callStrategy;
  strategies["CMD"] = &cmdStrategy;
  strategies["MAXFILES"] = &maxfilesStrategy;
  strategies["OPEN"] = &openStrategy;
  strategies["OPEN_GRP"] = &openGrpStrategy;
  strategies["CLOSE"] = &closeStrategy;
  strategies["BLOAD"] = &bloadStrategy;

  strategies["PLAY"] = &playStrategy;
  strategies["DRAW"] = &drawStrategy;
  strategies["LOCATE"] = &locateStrategy;
  strategies["SCREEN"] = &screenStrategy;
  strategies["WIDTH"] = &widthStrategy;
  strategies["COLOR"] = &colorStrategy;
  strategies["PSET"] = &psetStrategy;
  strategies["PRESET"] = &presetStrategy;
  strategies["LINE"] = &lineStrategy;
  strategies["PAINT"] = &paintStrategy;
  strategies["CIRCLE"] = &circleStrategy;
  strategies["COPY"] = &copyStrategy;
  strategies["PUT"] = &putStrategy;
  strategies["GET"] = &getStrategy;
  strategies["SET"] = &setStrategy;
  strategies["KEY"] = &keyStrategy;
  strategies["STRIG"] = &strigStrategy;
  strategies["SPRITE"] = &spriteStrategy;
}

ICompilerStatementStrategy* CompilerStatementStrategyFactory::getByKeyword(
    const string& keyword) {
  auto it = strategies.find(keyword);

  if (it == strategies.end()) return nullptr;

  return it->second;
}
