#ifndef COMPILER_STATEMENT_STRATEGY_FACTORY_H_INCLUDED
#define COMPILER_STATEMENT_STRATEGY_FACTORY_H_INCLUDED

#include <map>
#include <string>

#include "compiler_beep_statement_strategy.h"
#include "compiler_bload_statement_strategy.h"
#include "compiler_call_statement_strategy.h"
#include "compiler_circle_statement_strategy.h"
#include "compiler_clear_statement_strategy.h"
#include "compiler_close_statement_strategy.h"
#include "compiler_cls_statement_strategy.h"
#include "compiler_cmd_statement_strategy.h"
#include "compiler_color_statement_strategy.h"
#include "compiler_copy_statement_strategy.h"
#include "compiler_data_statement_strategy.h"
#include "compiler_def_statement_strategy.h"
#include "compiler_defdbl_statement_strategy.h"
#include "compiler_defint_statement_strategy.h"
#include "compiler_defsng_statement_strategy.h"
#include "compiler_defstr_statement_strategy.h"
#include "compiler_defusr_statement_strategy.h"
#include "compiler_dim_statement_strategy.h"
#include "compiler_draw_statement_strategy.h"
#include "compiler_end_statement_strategy.h"
#include "compiler_file_statement_strategy.h"
#include "compiler_for_statement_strategy.h"
#include "compiler_get_statement_strategy.h"
#include "compiler_gosub_statement_strategy.h"
#include "compiler_goto_statement_strategy.h"
#include "compiler_idata_statement_strategy.h"
#include "compiler_if_statement_strategy.h"
#include "compiler_input_statement_strategy.h"
#include "compiler_interval_statement_strategy.h"
#include "compiler_ipoke_statement_strategy.h"
#include "compiler_iread_statement_strategy.h"
#include "compiler_irestore_statement_strategy.h"
#include "compiler_key_statement_strategy.h"
#include "compiler_let_statement_strategy.h"
#include "compiler_line_statement_strategy.h"
#include "compiler_locate_statement_strategy.h"
#include "compiler_maxfiles_statement_strategy.h"
#include "compiler_next_statement_strategy.h"
#include "compiler_noop_statement_strategy.h"
#include "compiler_on_statement_strategy.h"
#include "compiler_open_grp_statement_strategy.h"
#include "compiler_open_statement_strategy.h"
#include "compiler_out_statement_strategy.h"
#include "compiler_paint_statement_strategy.h"
#include "compiler_play_statement_strategy.h"
#include "compiler_poke_statement_strategy.h"
#include "compiler_preset_statement_strategy.h"
#include "compiler_print_statement_strategy.h"
#include "compiler_pset_statement_strategy.h"
#include "compiler_put_statement_strategy.h"
#include "compiler_randomize_statement_strategy.h"
#include "compiler_read_statement_strategy.h"
#include "compiler_redim_statement_strategy.h"
#include "compiler_restore_statement_strategy.h"
#include "compiler_resume_statement_strategy.h"
#include "compiler_return_statement_strategy.h"
#include "compiler_screen_statement_strategy.h"
#include "compiler_set_statement_strategy.h"
#include "compiler_sound_statement_strategy.h"
#include "compiler_sprite_statement_strategy.h"
#include "compiler_statement_strategy.h"
#include "compiler_stop_statement_strategy.h"
#include "compiler_strig_statement_strategy.h"
#include "compiler_swap_statement_strategy.h"
#include "compiler_text_statement_strategy.h"
#include "compiler_time_statement_strategy.h"
#include "compiler_vpoke_statement_strategy.h"
#include "compiler_wait_statement_strategy.h"
#include "compiler_width_statement_strategy.h"

using namespace std;

class CompilerStatementStrategyFactory {
 private:
  NoopCompilerStatementStrategy noopStrategy;
  CompilerClearStatementStrategy clearStrategy;
  CompilerDefStatementStrategy defStrategy;
  CompilerDefintStatementStrategy defintStrategy;
  CompilerDefsngStatementStrategy defsngStrategy;
  CompilerDefdblStatementStrategy defdblStrategy;
  CompilerDefstrStatementStrategy defstrStrategy;
  CompilerDefUsrStatementStrategy defusrStrategy;
  CompilerClsStatementStrategy clsStrategy;
  CompilerBeepStatementStrategy beepStrategy;
  CompilerLetStatementStrategy letStrategy;
  CompilerTimeStatementStrategy timeStrategy;
  CompilerDimStatementStrategy dimStrategy;
  CompilerRedimStatementStrategy redimStrategy;
  CompilerRandomizeStatementStrategy randomizeStrategy;
  CompilerEndStatementStrategy endStrategy;
  CompilerGotoStatementStrategy gotoStrategy;
  CompilerGosubStatementStrategy gosubStrategy;
  CompilerReturnStatementStrategy returnStrategy;
  CompilerIfStatementStrategy ifStrategy;
  CompilerForStatementStrategy forStrategy;
  CompilerNextStatementStrategy nextStrategy;
  CompilerOnStatementStrategy onStrategy;
  CompilerIntervalStatementStrategy intervalStrategy;
  CompilerStopStatementStrategy stopStrategy;
  CompilerPrintStatementStrategy printStrategy;
  CompilerInputStatementStrategy inputStrategy;
  CompilerSoundStatementStrategy soundStrategy;
  CompilerOutStatementStrategy outStrategy;
  CompilerPokeStatementStrategy pokeStrategy;
  CompilerVpokeStatementStrategy vpokeStrategy;
  CompilerIpokeStatementStrategy ipokeStrategy;
  CompilerDataStatementStrategy dataStrategy;
  CompilerIdataStatementStrategy idataStrategy;
  CompilerReadStatementStrategy readStrategy;
  CompilerIreadStatementStrategy ireadStrategy;
  CompilerRestoreStatementStrategy restoreStrategy;
  CompilerIrestoreStatementStrategy irestoreStrategy;
  CompilerResumeStatementStrategy resumeStrategy;
  CompilerWaitStatementStrategy waitStrategy;
  CompilerSwapStatementStrategy swapStrategy;
  CompilerCallStatementStrategy callStrategy;
  CompilerCmdStatementStrategy cmdStrategy;
  CompilerMaxfilesStatementStrategy maxfilesStrategy;
  CompilerOpenStatementStrategy openStrategy;
  CompilerOpenGrpStatementStrategy openGrpStrategy;
  CompilerCloseStatementStrategy closeStrategy;
  CompilerFileStatementStrategy fileStrategy;
  CompilerTextStatementStrategy textStrategy;
  CompilerBloadStatementStrategy bloadStrategy;
  CompilerPlayStatementStrategy playStrategy;
  CompilerDrawStatementStrategy drawStrategy;
  CompilerLocateStatementStrategy locateStrategy;
  CompilerScreenStatementStrategy screenStrategy;
  CompilerWidthStatementStrategy widthStrategy;
  CompilerColorStatementStrategy colorStrategy;
  CompilerPsetStatementStrategy psetStrategy;
  CompilerPresetStatementStrategy presetStrategy;
  CompilerLineStatementStrategy lineStrategy;
  CompilerPaintStatementStrategy paintStrategy;
  CompilerCircleStatementStrategy circleStrategy;
  CompilerCopyStatementStrategy copyStrategy;
  CompilerPutStatementStrategy putStrategy;
  CompilerGetStatementStrategy getStrategy;
  CompilerSetStatementStrategy setStrategy;
  CompilerKeyStatementStrategy keyStrategy;
  CompilerStrigStatementStrategy strigStrategy;
  CompilerSpriteStatementStrategy spriteStrategy;
  map<string, ICompilerStatementStrategy*> strategies;

 public:
  CompilerStatementStrategyFactory();
  ICompilerStatementStrategy* getStrategyByKeyword(const string& keyword);
};

#endif  // COMPILER_STATEMENT_STRATEGY_FACTORY_H_INCLUDED
