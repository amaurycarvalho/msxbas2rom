; ------------------------------------------------------------------------------------------------------
; MSXBAS2ROM Z80 header routines (for compiled mode)
; by: Amaury Carvalho, 2020-2026
; Compile with:
;   pasmo header.asm header.bin
;   xxd -i header.bin header.h
; ------------------------------------------------------------------------------------------------------
;
; This file orchestrates domain-specific modules  
; improving maintainability through SRP boundaries.

include "src/header/00_constants_and_workarea.asm"
include "src/header/10_macros.asm"
include "src/header/20_runtime.asm"
include "src/header/21_logic_pack.asm"
include "src/header/30_basic_helpers.asm"
include "src/header/31_cmd.asm"
include "src/header/32_usr.asm"
include "src/header/33_player.asm"
include "src/header/60_bios_helpers.asm"
include "src/header/61_megarom.asm"
include "src/header/70_pletter.asm"
include "src/header/80_resources.asm"
include "src/header/90_support.asm"
