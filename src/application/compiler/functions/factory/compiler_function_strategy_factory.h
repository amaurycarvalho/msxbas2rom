#ifndef COMPILER_FUNCTION_STRATEGY_FACTORY_H_INCLUDED
#define COMPILER_FUNCTION_STRATEGY_FACTORY_H_INCLUDED

#include <map>
#include <string>

#include "compiler_abs_function_strategy.h"
#include "compiler_asc_function_strategy.h"
#include "compiler_atn_function_strategy.h"
#include "compiler_base_function_strategy.h"
#include "compiler_bin_function_strategy.h"
#include "compiler_cdbl_function_strategy.h"
#include "compiler_chr_function_strategy.h"
#include "compiler_cint_function_strategy.h"
#include "compiler_collision_function_strategy.h"
#include "compiler_cos_function_strategy.h"
#include "compiler_csng_function_strategy.h"
#include "compiler_csrlin_function_strategy.h"
#include "compiler_eof_function_strategy.h"
#include "compiler_exp_function_strategy.h"
#include "compiler_fix_function_strategy.h"
#include "compiler_fre_function_strategy.h"
#include "compiler_function_strategy.h"
#include "compiler_heap_function_strategy.h"
#include "compiler_hex_function_strategy.h"
#include "compiler_inkey_function_strategy.h"
#include "compiler_inkey_string_function_strategy.h"
#include "compiler_inp_function_strategy.h"
#include "compiler_input_string_function_strategy.h"
#include "compiler_instr_function_strategy.h"
#include "compiler_int_function_strategy.h"
#include "compiler_ipeek_function_strategy.h"
#include "compiler_left_function_strategy.h"
#include "compiler_len_function_strategy.h"
#include "compiler_log_function_strategy.h"
#include "compiler_lpos_function_strategy.h"
#include "compiler_maker_function_strategy.h"
#include "compiler_maxfiles_function_strategy.h"
#include "compiler_mid_function_strategy.h"
#include "compiler_msx_function_strategy.h"
#include "compiler_noop_function_strategy.h"
#include "compiler_ntsc_function_strategy.h"
#include "compiler_oct_function_strategy.h"
#include "compiler_pad_function_strategy.h"
#include "compiler_pdl_function_strategy.h"
#include "compiler_peek_function_strategy.h"
#include "compiler_play_function_strategy.h"
#include "compiler_plystatus_function_strategy.h"
#include "compiler_point_function_strategy.h"
#include "compiler_pos_function_strategy.h"
#include "compiler_psg_function_strategy.h"
#include "compiler_resource_function_strategy.h"
#include "compiler_resourcesize_function_strategy.h"
#include "compiler_right_function_strategy.h"
#include "compiler_rnd_function_strategy.h"
#include "compiler_sgn_function_strategy.h"
#include "compiler_sin_function_strategy.h"
#include "compiler_space_function_strategy.h"
#include "compiler_sqr_function_strategy.h"
#include "compiler_stick_function_strategy.h"
#include "compiler_str_function_strategy.h"
#include "compiler_strig_function_strategy.h"
#include "compiler_string_function_strategy.h"
#include "compiler_tab_function_strategy.h"
#include "compiler_tan_function_strategy.h"
#include "compiler_tile_function_strategy.h"
#include "compiler_time_function_strategy.h"
#include "compiler_turbo_function_strategy.h"
#include "compiler_using_function_strategy.h"
#include "compiler_usr_function_strategy.h"
#include "compiler_val_function_strategy.h"
#include "compiler_vdp_function_strategy.h"
#include "compiler_vpeek_function_strategy.h"
#include "compiler_varptr_function_strategy.h"

using namespace std;

class CompilerFunctionStrategyFactory {
 private:
  EofCompilerFunctionStrategy eofStrategy;
  FreCompilerFunctionStrategy freStrategy;
  HeapCompilerFunctionStrategy heapStrategy;
  InkeyCompilerFunctionStrategy inkeyStrategy;
  InkeyStringCompilerFunctionStrategy inkeyStringStrategy;
  InpCompilerFunctionStrategy inpStrategy;
  InputStringCompilerFunctionStrategy inputStringStrategy;
  IpeekCompilerFunctionStrategy ipeekStrategy;
  AbsCompilerFunctionStrategy absStrategy;
  AtnCompilerFunctionStrategy atnStrategy;
  AscCompilerFunctionStrategy ascStrategy;
  BaseCompilerFunctionStrategy baseStrategy;
  BinCompilerFunctionStrategy binStrategy;
  CdblCompilerFunctionStrategy cdblStrategy;
  CintCompilerFunctionStrategy cintStrategy;
  CsngCompilerFunctionStrategy csngStrategy;
  CsrlinCompilerFunctionStrategy csrlinStrategy;
  ChrCompilerFunctionStrategy chrStrategy;
  CollisionCompilerFunctionStrategy collisionStrategy;
  CosCompilerFunctionStrategy cosStrategy;
  ExpCompilerFunctionStrategy expStrategy;
  HexCompilerFunctionStrategy hexStrategy;
  InstrCompilerFunctionStrategy instrStrategy;
  LeftCompilerFunctionStrategy leftStrategy;
  LenCompilerFunctionStrategy lenStrategy;
  MidCompilerFunctionStrategy midStrategy;
  OctCompilerFunctionStrategy octStrategy;
  RightCompilerFunctionStrategy rightStrategy;
  SpaceCompilerFunctionStrategy spaceStrategy;
  StrCompilerFunctionStrategy strStrategy;
  StringCompilerFunctionStrategy stringStrategy;
  TabCompilerFunctionStrategy tabStrategy;
  MaxfilesCompilerFunctionStrategy maxfilesStrategy;
  FixCompilerFunctionStrategy fixStrategy;
  IntCompilerFunctionStrategy intStrategy;
  LogCompilerFunctionStrategy logStrategy;
  MakerCompilerFunctionStrategy makerStrategy;
  MsxCompilerFunctionStrategy msxStrategy;
  NtscCompilerFunctionStrategy ntscStrategy;
  NoopCompilerFunctionStrategy noopStrategy;
  PosCompilerFunctionStrategy posStrategy;
  RndCompilerFunctionStrategy rndStrategy;
  PadCompilerFunctionStrategy padStrategy;
  PlayCompilerFunctionStrategy playStrategy;
  PointCompilerFunctionStrategy pointStrategy;
  PdlCompilerFunctionStrategy pdlStrategy;
  PsgCompilerFunctionStrategy psgStrategy;
  PlystatusCompilerFunctionStrategy plystatusStrategy;
  PeekCompilerFunctionStrategy peekStrategy;
  ResourceCompilerFunctionStrategy resourceStrategy;
  ResourcesizeCompilerFunctionStrategy resourcesizeStrategy;
  SgnCompilerFunctionStrategy sgnStrategy;
  SinCompilerFunctionStrategy sinStrategy;
  SqrCompilerFunctionStrategy sqrStrategy;
  StickCompilerFunctionStrategy stickStrategy;
  StrigCompilerFunctionStrategy strigStrategy;
  TileCompilerFunctionStrategy tileStrategy;
  TanCompilerFunctionStrategy tanStrategy;
  TurboCompilerFunctionStrategy turboStrategy;
  TimeCompilerFunctionStrategy timeStrategy;
  VdpCompilerFunctionStrategy vdpStrategy;
  VpeekCompilerFunctionStrategy vpeekStrategy;
  UsingCompilerFunctionStrategy usingStrategy;
  VarptrCompilerFunctionStrategy varptrStrategy;
  UsrCompilerFunctionStrategy usrStrategy;
  ValCompilerFunctionStrategy valStrategy;
  LposCompilerFunctionStrategy lposStrategy;

  map<string, ICompilerFunctionStrategy*> strategies;

 public:
  CompilerFunctionStrategyFactory();
  ICompilerFunctionStrategy* getByKeyword(const string& keyword);
};

#endif  // COMPILER_FUNCTION_STRATEGY_FACTORY_H_INCLUDED
