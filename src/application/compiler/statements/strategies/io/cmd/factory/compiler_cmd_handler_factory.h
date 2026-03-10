#ifndef COMPILER_CMD_HANDLER_FACTORY_H_INCLUDED
#define COMPILER_CMD_HANDLER_FACTORY_H_INCLUDED

#include <map>
#include <string>

#include "compiler_cmd_clip_handler.h"
#include "compiler_cmd_clrkey_handler.h"
#include "compiler_cmd_clrscr_handler.h"
#include "compiler_cmd_disscr_handler.h"
#include "compiler_cmd_draw_handler.h"
#include "compiler_cmd_enascr_handler.h"
#include "compiler_cmd_handler.h"
#include "compiler_cmd_keyclkoff_handler.h"
#include "compiler_cmd_mtf_handler.h"
#include "compiler_cmd_mute_handler.h"
#include "compiler_cmd_page_handler.h"
#include "compiler_cmd_play_handler.h"
#include "compiler_cmd_plyload_handler.h"
#include "compiler_cmd_plyloop_handler.h"
#include "compiler_cmd_plymute_handler.h"
#include "compiler_cmd_plyplay_handler.h"
#include "compiler_cmd_plyreplay_handler.h"
#include "compiler_cmd_plysong_handler.h"
#include "compiler_cmd_plysound_handler.h"
#include "compiler_cmd_pt3_handler.h"
#include "compiler_cmd_ramtoram_handler.h"
#include "compiler_cmd_ramtovram_handler.h"
#include "compiler_cmd_restore_handler.h"
#include "compiler_cmd_rsctoram_handler.h"
#include "compiler_cmd_runasm_handler.h"
#include "compiler_cmd_runbas_handler.h"
#include "compiler_cmd_setfnt_handler.h"
#include "compiler_cmd_turbo_handler.h"
#include "compiler_cmd_updfntclr_handler.h"
#include "compiler_cmd_vramtoram_handler.h"
#include "compiler_cmd_wrtchr_handler.h"
#include "compiler_cmd_wrtclr_handler.h"
#include "compiler_cmd_wrtfnt_handler.h"
#include "compiler_cmd_wrtscr_handler.h"
#include "compiler_cmd_wrtspr_handler.h"
#include "compiler_cmd_wrtspratr_handler.h"
#include "compiler_cmd_wrtsprclr_handler.h"
#include "compiler_cmd_wrtsprpat_handler.h"
#include "compiler_cmd_wrtvram_handler.h"

using namespace std;

class CompilerCmdHandlerFactory {
 private:
  CompilerCmdPlayHandler playHandler;
  CompilerCmdRunAsmHandler runasmHandler;
  CompilerCmdRunBasHandler runbasHandler;
  CompilerCmdWrtFntHandler wrtfntHandler;
  CompilerCmdWrtChrHandler wrtchrHandler;
  CompilerCmdWrtClrHandler wrtclrHandler;
  CompilerCmdWrtScrHandler wrtscrHandler;
  CompilerCmdWrtSprHandler wrtsprHandler;
  CompilerCmdWrtSprPatHandler wrtsprpatHandler;
  CompilerCmdWrtSprClrHandler wrtsprclrHandler;
  CompilerCmdWrtSprAtrHandler wrtspratrHandler;
  CompilerCmdWrtVramHandler wrtvramHandler;
  CompilerCmdRamToVramHandler ramtovramHandler;
  CompilerCmdVramToRamHandler vramtoramHandler;
  CompilerCmdRamToRamHandler ramtoramHandler;
  CompilerCmdRscToRamHandler rsctoramHandler;
  CompilerCmdDisScrHandler disscrHandler;
  CompilerCmdEnaScrHandler enascrHandler;
  CompilerCmdKeyClkOffHandler keyclkoffHandler;
  CompilerCmdMuteHandler muteHandler;
  CompilerCmdDrawHandler drawHandler;
  CompilerCmdClipHandler clipHandler;
  CompilerCmdPt3Handler pt3Handler;
  CompilerCmdPlyLoadHandler plyloadHandler;
  CompilerCmdPlyPlayHandler plyplayHandler;
  CompilerCmdPlySongHandler plysongHandler;
  CompilerCmdPlyMuteHandler plymuteHandler;
  CompilerCmdPlyLoopHandler plyloopHandler;
  CompilerCmdPlyReplayHandler plyreplayHandler;
  CompilerCmdPlySoundHandler plysoundHandler;
  CompilerCmdMtfHandler mtfHandler;
  CompilerCmdSetFntHandler setfntHandler;
  CompilerCmdUpdFntClrHandler updfntclrHandler;
  CompilerCmdClrScrHandler clrscrHandler;
  CompilerCmdClrKeyHandler clrkeyHandler;
  CompilerCmdPageHandler pageHandler;
  CompilerCmdRestoreHandler restoreHandler;
  CompilerCmdTurboHandler turboHandler;

  map<string, ICompilerCmdHandler*> handlers;

 public:
  CompilerCmdHandlerFactory();
  ICompilerCmdHandler* getByKeyword(const string& keyword);
};

#endif  // COMPILER_CMD_HANDLER_FACTORY_H_INCLUDED
