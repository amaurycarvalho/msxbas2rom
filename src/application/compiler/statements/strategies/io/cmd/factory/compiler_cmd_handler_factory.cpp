/***
 * @file compiler_cmd_handler_factory.cpp
 * @brief Compiler CMD handler factory
 */

#include "compiler_cmd_handler_factory.h"

CompilerCmdHandlerFactory::CompilerCmdHandlerFactory() {
  handlers["RUNASM"] = &runasmHandler;
  handlers["RUNBAS"] = &runbasHandler;
  handlers["WRTVRAM"] = &wrtvramHandler;
  handlers["WRTFNT"] = &wrtfntHandler;
  handlers["WRTCHR"] = &wrtchrHandler;
  handlers["WRTCLR"] = &wrtclrHandler;
  handlers["WRTSCR"] = &wrtscrHandler;
  handlers["WRTSPR"] = &wrtsprHandler;
  handlers["WRTSPRPAT"] = &wrtsprpatHandler;
  handlers["WRTSPRCLR"] = &wrtsprclrHandler;
  handlers["WRTSPRATR"] = &wrtspratrHandler;
  handlers["RAMTOVRAM"] = &ramtovramHandler;
  handlers["VRAMTORAM"] = &vramtoramHandler;
  handlers["RAMTORAM"] = &ramtoramHandler;
  handlers["RSCTORAM"] = &rsctoramHandler;
  handlers["DISSCR"] = &disscrHandler;
  handlers["ENASCR"] = &enascrHandler;
  handlers["KEYCLKOFF"] = &keyclkoffHandler;
  handlers["MUTE"] = &muteHandler;
  handlers["PLAY"] = &playHandler;
  handlers["DRAW"] = &drawHandler;
  handlers["CLIP"] = &clipHandler;
  handlers["PT3LOAD"] = &pt3Handler;
  handlers["PT3PLAY"] = &pt3Handler;
  handlers["PT3MUTE"] = &pt3Handler;
  handlers["PT3LOOP"] = &pt3Handler;
  handlers["PT3REPLAY"] = &pt3Handler;
  handlers["PLYLOAD"] = &plyloadHandler;
  handlers["PLYPLAY"] = &plyplayHandler;
  handlers["PLYSONG"] = &plysongHandler;
  handlers["PLYMUTE"] = &plymuteHandler;
  handlers["PLYLOOP"] = &plyloopHandler;
  handlers["PLYREPLAY"] = &plyreplayHandler;
  handlers["PLYSOUND"] = &plysoundHandler;
  handlers["MTF"] = &mtfHandler;
  handlers["SETFNT"] = &setfntHandler;
  handlers["UPDFNTCLR"] = &updfntclrHandler;
  handlers["CLRSCR"] = &clrscrHandler;
  handlers["CLRKEY"] = &clrkeyHandler;
  handlers["PAGE"] = &pageHandler;
  handlers["RESTORE"] = &restoreHandler;
  handlers["TURBO"] = &turboHandler;
}

ICompilerCmdHandler* CompilerCmdHandlerFactory::getByKeyword(
    const string& keyword) {
  auto it = handlers.find(keyword);

  if (it == handlers.end()) return nullptr;

  return it->second;
}
