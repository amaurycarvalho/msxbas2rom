/***
 * @file compiler_function_strategy_factory.cpp
 * @brief Compiler function strategy factory
 */

#include "compiler_function_strategy_factory.h"

CompilerFunctionStrategyFactory::CompilerFunctionStrategyFactory() {
  strategies["NOOPERATION"] = &noopStrategy;

  strategies["TIME"] = &timeStrategy;
  strategies["POS"] = &posStrategy;
  strategies["LPOS"] = &lposStrategy;
  strategies["CSRLIN"] = &csrlinStrategy;
  strategies["INKEY"] = &inkeyStrategy;
  strategies["INKEY$"] = &inkeyStringStrategy;
  strategies["MAXFILES"] = &maxfilesStrategy;
  strategies["FRE"] = &freStrategy;
  strategies["HEAP"] = &heapStrategy;
  strategies["MSX"] = &msxStrategy;
  strategies["NTSC"] = &ntscStrategy;
  strategies["VDP"] = &vdpStrategy;
  strategies["TURBO"] = &turboStrategy;
  strategies["COLLISION"] = &collisionStrategy;
  strategies["MAKER"] = &makerStrategy;
  strategies["PLYSTATUS"] = &plystatusStrategy;

  strategies["INT"] = &intStrategy;
  strategies["FIX"] = &fixStrategy;
  strategies["RND"] = &rndStrategy;
  strategies["SIN"] = &sinStrategy;
  strategies["COS"] = &cosStrategy;
  strategies["TAN"] = &tanStrategy;
  strategies["ATN"] = &atnStrategy;
  strategies["EXP"] = &expStrategy;
  strategies["LOG"] = &logStrategy;
  strategies["SQR"] = &sqrStrategy;
  strategies["SGN"] = &sgnStrategy;
  strategies["ABS"] = &absStrategy;
  strategies["VAL"] = &valStrategy;
  strategies["PEEK"] = &peekStrategy;
  strategies["IPEEK"] = &ipeekStrategy;
  strategies["VPEEK"] = &vpeekStrategy;
  strategies["INP"] = &inpStrategy;
  strategies["EOF"] = &eofStrategy;
  strategies["PSG"] = &psgStrategy;
  strategies["PLAY"] = &playStrategy;
  strategies["STICK"] = &stickStrategy;
  strategies["STRIG"] = &strigStrategy;
  strategies["PAD"] = &padStrategy;
  strategies["PDL"] = &pdlStrategy;
  strategies["BASE"] = &baseStrategy;
  strategies["ASC"] = &ascStrategy;
  strategies["LEN"] = &lenStrategy;
  strategies["CSNG"] = &csngStrategy;
  strategies["CDBL"] = &cdblStrategy;
  strategies["CINT"] = &cintStrategy;
  strategies["CHR$"] = &chrStrategy;
  strategies["SPACE$"] = &spaceStrategy;
  strategies["SPC"] = &spaceStrategy;
  strategies["TAB"] = &tabStrategy;
  strategies["STR$"] = &strStrategy;
  strategies["INPUT$"] = &inputStringStrategy;
  strategies["BIN$"] = &binStrategy;
  strategies["OCT$"] = &octStrategy;
  strategies["HEX$"] = &hexStrategy;
  strategies["RESOURCE"] = &resourceStrategy;
  strategies["RESOURCESIZE"] = &resourcesizeStrategy;
  strategies["USR"] = &usrStrategy;
  strategies["USR0"] = &usrStrategy;
  strategies["USR1"] = &usrStrategy;
  strategies["USR2"] = &usrStrategy;
  strategies["USR3"] = &usrStrategy;
  strategies["USR4"] = &usrStrategy;
  strategies["USR5"] = &usrStrategy;
  strategies["USR6"] = &usrStrategy;
  strategies["USR7"] = &usrStrategy;
  strategies["USR8"] = &usrStrategy;
  strategies["USR9"] = &usrStrategy;

  strategies["POINT"] = &pointStrategy;
  strategies["TILE"] = &tileStrategy;
  strategies["STRING$"] = &stringStrategy;
  strategies["LEFT$"] = &leftStrategy;
  strategies["RIGHT$"] = &rightStrategy;
  strategies["MID$"] = &midStrategy;
  strategies["USING$"] = &usingStrategy;
  strategies["INSTR"] = &instrStrategy;
  strategies["VARPTR"] = &varptrStrategy;
}

ICompilerFunctionStrategy* CompilerFunctionStrategyFactory::getByKeyword(
    const string& keyword) {
  auto it = strategies.find(keyword);

  if (it == strategies.end()) return nullptr;

  return it->second;
}

size_t CompilerFunctionStrategyFactory::size() const {
  return strategies.size();
}