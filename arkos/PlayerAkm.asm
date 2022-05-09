;       Arkos Tracker 2 AKM (Minimalist) player (format V0).
;       By Targhan/Arkos.
;
;       Thanks to Hicks/Vanity for two small (but relevant!) optimizations.

;       This compiles with RASM. Check the compatibility page on the Arkos Tracker 2 website, it contains a source converter to any Z80 assembler!;

;       This is a Minimalist player. Only a subset of the generic player is used. Use this player for 4k demo or other productions
;       with a tight memory limitation. However, this remains a versatile and powerful player, so it may fit any production!
;
;       Though the player is optimized in speed, it is much slower than the generic one or the AKY player.
;       With effects used at the same time, it can reach 45 scanlines on a CPC, plus some few more if you are using sound effects.
;       So it's about as fast as the Soundtrakker 128 player, but smaller and more powerful (so what are you complaining about?).
;
;       The player uses the stack for optimizations. Make sure the interruptions are disabled before it is called.
;       The stack pointer is saved at the beginning and restored at the end.
;
;       Target hardware:
;       ---------------
;       This code can target Amstrad CPC, MSX, Spectrum and Pentagon. By default, it targets Amstrad CPC.
;       Simply use one of the follow line (BEFORE this player):
;       PLY_AKM_HARDWARE_CPC = 1
;       PLY_AKM_HARDWARE_MSX = 1
;       PLY_AKM_HARDWARE_SPECTRUM = 1
;       PLY_AKM_HARDWARE_PENTAGON = 1
;       Note that the PRESENCE of this variable is tested, NOT its value.

;       Some severe optimizations of CPU/memory can be performed:
;       ---------------------------------------------------------
;       - Use the Player Configuration of Arkos Tracker 2 to generate a configuration file to be included at the beginning of this player.
;         It will disable useless features according to your songs! Check the manual for more details, or more simply the testers.

;       Sound effects:
;       --------------
;       Sound effects are disabled by default. Declare PLY_AKM_MANAGE_SOUND_EFFECTS to enable it:
;       PLY_AKM_MANAGE_SOUND_EFFECTS = 1
;       Check the sound effect tester to see how it enables it.
;       Note that the PRESENCE of this variable is tested, NOT its value.
;
;       ROM
;       ----------------------
;       To use a ROM player (no automodification, use of a small buffer to put in RAM):
;       PLY_AKM_Rom = 1
;       PLY_AKM_ROM_Buffer = #C100 (or wherever).
;       This makes the player a bit slower and slightly bigger.
;       The buffer is PLY_AKM_ROM_BufferSize bytes long (199 bytes max).
;
;       -------------------------------------------------------
PLY_AKM_Start:


        ;Checks the hardware. Only one must be selected.
PLY_AKM_HardwareCounter = 0
        IFDEF PLY_AKM_HARDWARE_CPC
                PLY_AKM_HardwareCounter = PLY_AKM_HardwareCounter + 1
        ENDIF
        IFDEF PLY_AKM_HARDWARE_MSX
                PLY_AKM_HardwareCounter = PLY_AKM_HardwareCounter + 1
                PLY_AKM_HARDWARE_SPECTRUM_OR_MSX = 1
        ENDIF
        IFDEF PLY_AKM_HARDWARE_SPECTRUM
                PLY_AKM_HardwareCounter = PLY_AKM_HardwareCounter + 1
                PLY_AKM_HARDWARE_SPECTRUM_OR_PENTAGON = 1
                PLY_AKM_HARDWARE_SPECTRUM_OR_MSX = 1
        ENDIF
        IFDEF PLY_AKM_HARDWARE_PENTAGON
                PLY_AKM_HardwareCounter = PLY_AKM_HardwareCounter + 1
                PLY_AKM_HARDWARE_SPECTRUM_OR_PENTAGON = 1
        ENDIF
        IF PLY_AKM_HARDWARECounter > 1
                FAIL 'Only one hardware must be selected!'
        ENDIF
        ;By default, selects the Amstrad CPC.
        IF PLY_AKM_HARDWARECounter == 0
                PLY_AKM_HARDWARE_CPC = 1
        ENDIF


        ;Disark macro: Word region Start.
        disarkCounter = 0
        IFNDEF dkws
        MACRO dkws
PLY_AKM_DisarkWordRegionStart_{disarkCounter}
        ENDM
        ENDIF
        ;Disark macro: Word region End.
        IFNDEF dkwe
        MACRO dkwe
PLY_AKM_DisarkWordRegionEnd_{disarkCounter}:
        disarkCounter = disarkCounter + 1
        ENDM
        ENDIF
        
        ;Disark macro: Pointer region Start.
        disarkCounter = 0
        IFNDEF dkps
        MACRO dkps
PLY_AKM_DisarkPointerRegionStart_{disarkCounter}
        ENDM
        ENDIF
        ;Disark macro: Pointer region End.
        IFNDEF dkpe
        MACRO dkpe
PLY_AKM_DisarkPointerRegionEnd_{disarkCounter}:
        disarkCounter = disarkCounter + 1
        ENDM
        ENDIF
        
        ;Disark macro: Byte region Start.
        disarkCounter = 0
        IFNDEF dkbs
        MACRO dkbs
PLY_AKM_DisarkByteRegionStart_{disarkCounter}
        ENDM
        ENDIF
        ;Disark macro: Byte region End.
        IFNDEF dkbe
        MACRO dkbe
PLY_AKM_DisarkByteRegionEnd_{disarkCounter}:
        disarkCounter = disarkCounter + 1
        ENDM
        ENDIF

        ;Disark macro: Force "No Reference Area" for 3 bytes (ld hl,xxxx).
        IFNDEF dknr3
        MACRO dknr3
PLY_AKM_DisarkForceNonReferenceDuring3_{disarkCounter}:
        disarkCounter = disarkCounter + 1
        ENDM
        ENDIF
        

PLY_AKM_USE_HOOKS: equ 1                                 ;Use hooks for external calls? 0 if the Init/Play methods are directly called, will save a few bytes.
PLY_AKM_STOP_SOUNDS: equ 1                               ;1 to have the "stop sounds" code. Set it to 0 if you never plan on stopping your music.
        
        ;Hooks for external calls. Can be removed if not needed.
        if PLY_AKM_USE_HOOKS
		assert PLY_AKM_Start == $		;Makes sure no extra byte were inserted before the hooks.
                jp PLY_AKM_Init          ;Player + 0.
                jp PLY_AKM_Play          ;Player + 3.
                if PLY_AKM_STOP_SOUNDS
                jp PLY_AKM_Stop          ;Player + 6.
                endif
        endif
        
        ;Includes the sound effects player, if wanted. Important to do it as soon as possible, so that
        ;its code can react to the Player Configuration and possibly alter it.
        IFDEF PLY_AKM_MANAGE_SOUND_EFFECTS
		include "PlayerAkm_SoundEffects.asm"
        ENDIF
        ;[[INSERT_SOUND_EFFECT_SOURCE]]                 ;A tag for test units. Don't touch or you're dead.

        ;Is there a loaded Player Configuration source? If no, use a default configuration.
        IFNDEF PLY_CFG_ConfigurationIsPresent
                PLY_CFG_UseTranspositions = 1
                PLY_CFG_UseSpeedTracks = 1
                PLY_CFG_UseEffects = 1
                PLY_CFG_UseHardwareSounds = 1
                PLY_CFG_NoSoftNoHard_Noise = 1
                PLY_CFG_SoftOnly_Noise = 1
                PLY_CFG_SoftOnly_SoftwarePitch = 1
                PLY_CFG_SoftToHard_SoftwarePitch = 1
                PLY_CFG_SoftToHard_SoftwareArpeggio = 1
                PLY_CFG_SoftAndHard_SoftwarePitch = 1
                PLY_CFG_SoftAndHard_SoftwareArpeggio = 1
                PLY_CFG_UseEffect_ArpeggioTable = 1
                PLY_CFG_UseEffect_ForcePitchTableSpeed = 1
                PLY_CFG_UseEffect_ForceArpeggioSpeed = 1
                PLY_CFG_UseEffect_ForceInstrumentSpeed = 1
                PLY_CFG_UseEffect_PitchUp = 1
                PLY_CFG_UseEffect_PitchDown = 1
                PLY_CFG_UseEffect_PitchTable = 1
                PLY_CFG_UseEffect_SetVolume = 1
                PLY_CFG_UseEffect_Reset = 1
        ENDIF
        
        ;Agglomerates some flags, because they are treated the same way by this player.
        ;--------------------------------------------------
        ;Creates a flag for pitch in instrument, and also pitch in hardware.
        IFDEF PLY_CFG_SoftOnly_SoftwarePitch
                PLY_AKM_PitchInInstrument = 1
        ENDIF
        IFDEF PLY_CFG_SoftToHard_SoftwarePitch
                PLY_AKM_PitchInInstrument = 1
                PLY_AKM_PitchInHardwareInstrument = 1
        ENDIF
        IFDEF PLY_CFG_SoftAndHard_SoftwarePitch
                PLY_AKM_PitchInInstrument = 1
                PLY_AKM_PitchInHardwareInstrument = 1
        ENDIF
        ;A flag for Arpeggios in Instrument, both in software and hardware.
        IFDEF PLY_CFG_SoftOnly_SoftwareArpeggio
                PLY_AKM_ArpeggioInSoftwareOrHardwareInstrument = 1
        ENDIF
        IFDEF PLY_CFG_SoftToHard_SoftwareArpeggio
                PLY_AKM_ArpeggioInSoftwareOrHardwareInstrument = 1
                PLY_AKM_ArpeggioInHardwareInstrument = 1
        ENDIF
        IFDEF PLY_CFG_SoftAndHard_SoftwareArpeggio
                PLY_AKM_ArpeggioInSoftwareOrHardwareInstrument = 1
                PLY_AKM_ArpeggioInHardwareInstrument = 1
        ENDIF      
        
        ;A flag if noise is used (noise in hardware not tested, not present in this format).
        IFDEF PLY_CFG_NoSoftNoHard_Noise
                PLY_AKM_USE_Noise = 1
        ENDIF
        IFDEF PLY_CFG_SoftOnly_Noise
                PLY_AKM_USE_Noise = 1
        ENDIF
        ;The noise is managed? Then the noise register access must be compiled.
        IFDEF PLY_AKM_USE_Noise
                PLY_AKM_USE_NoiseRegister = 1
        ENDIF
        
        ;Mixing Pitch up/down effects.
        IFDEF PLY_CFG_UseEffect_PitchUp
                PLY_AKM_USE_EffectPitchUpDown = 1
        ENDIF
        IFDEF PLY_CFG_UseEffect_PitchDown
                PLY_AKM_USE_EffectPitchUpDown = 1
        ENDIF
        
        ;If the Force Arpeggio Speed if used, it means the ArpeggioTable effect must also be!
        IFDEF PLY_CFG_UseEffect_ForceArpeggioSpeed
                PLY_CFG_UseEffect_ArpeggioTable = 1
        ENDIF
        ;If the Force Pitch Table Speed if used, it means the PitchTable effect must also be!
        IFDEF PLY_CFG_UseEffect_ForcePitchTableSpeed
                PLY_CFG_UseEffect_PitchTable = 1
        ENDIF

;A nice trick to manage the offset using the same instructions, according to the player (ROM or not).
        IFDEF PLY_AKM_Rom
PLY_AKM_Offset1b: equ 0
PLY_AKM_Offset2b: equ 0         ;Used for instructions such as ld iyh,xx
        ELSE
PLY_AKM_Offset1b: equ 1
PLY_AKM_Offset2b: equ 2
        ENDIF

;Initializes the song. MUST be called before actually playing the song.
;IN:    HL = Address of the song.
;       A = Index of the subsong to play (>=0).
PLY_AKM_InitDisarkGenerateExternalLabel:
PLY_AKM_Init:
        ;Reads the Song header.
        ;Reads the pointers to the various index tables.
        ld de,PLY_AKM_PtInstruments + PLY_AKM_Offset1b
        ldi
        ldi
                        IFDEF PLY_CFG_UseEffects                           ;CONFIG SPECIFIC
                                IFDEF PLY_CFG_UseEffect_ArpeggioTable      ;CONFIG SPECIFIC
        ld de,PLY_AKM_PtArpeggios + PLY_AKM_Offset1b
        ldi
        ldi
                                ELSE
                                inc hl
                                inc hl
                                ENDIF ;PLY_CFG_UseEffect_ArpeggioTable
                                IFDEF PLY_CFG_UseEffect_PitchTable         ;CONFIG SPECIFIC
        ld de,PLY_AKM_PtPitches + PLY_AKM_Offset1b
        ldi
        ldi
                                ELSE
                                inc hl
                                inc hl
                                ENDIF ;PLY_CFG_UseEffect_PitchTable
                        ELSE
dknr3 (void):  ld de,4
        add hl,de
                        ENDIF ;PLY_CFG_UseEffects
                        
        ;Finds the address of the Subsong.
        ;HL points on the table, adds A * 2.
        ;Possible optimization: possible to set the Subsong directly.
        add a,a
        ld e,a
        ld d,0
        add hl,de
        ld a,(hl)
        inc hl
        ld h,(hl)
        ld l,a

        ;Reads the header of the Subsong, copies the values inside the code via a table.
        ld ix,PLY_AKM_InitVars_Start
        ld a,(PLY_AKM_InitVars_End - PLY_AKM_InitVars_Start) / 2
PLY_AKM_InitVars_Loop:
        ld e,(ix + 0)
        ld d,(ix + 1)
        inc ix
        inc ix
        ldi
        dec a
        jr nz,PLY_AKM_InitVars_Loop

        ;A is zero, no need to reset it.        
        ld (PLY_AKM_PatternRemainingHeight + PLY_AKM_Offset1b),a       ;Optimization: this line can be removed if there is no need to reset the song (warning, A is used below).

        ;Stores the Linker address, just after.
        ex de,hl
        ld hl,PLY_AKM_PtLinker + PLY_AKM_Offset1b
        ld (hl),e
        inc hl
        ld (hl),d

        ;A big LDIR to erase all the data blocks. Optimization: can be removed if there is no need to reset the song.
        ;A is considered 0!
        ld hl,PLY_AKM_Track1_Data
        ld de,PLY_AKM_Track1_Data + 1
dknr3 (void):  ld bc,PLY_AKM_Track3_Data_End - PLY_AKM_Track1_Data - 1
        ld (hl),a
        ldir
        
        ;Resets this flag. Especially important for ROM.
        IFDEF PLY_CFG_UseEffects        ;CONFIG SPECIFIC
                ld (PLY_AKM_RT_ReadEffectsFlag + PLY_AKM_Offset1b),a
        ENDIF

        ;Forces a new line.
        ld a,(PLY_AKM_Speed + PLY_AKM_Offset1b)
        dec a
        ld (PLY_AKM_TickCounter + PLY_AKM_Offset1b),a

        ;Reads the first instrument, the empty one, and set-ups the pointers to the instrument to read.
        ;Optimization: needed if the song doesn't start with an instrument on all the channels. Else, it can be removed.
        ld hl,(PLY_AKM_PtInstruments + PLY_AKM_Offset1b)
        ld e,(hl)
        inc hl
        ld d,(hl)
        inc de          ;Skips the header.
        ld (PLY_AKM_Track1_PtInstrument),de
        ld (PLY_AKM_Track2_PtInstrument),de
        ld (PLY_AKM_Track3_PtInstrument),de
        
        ;If sound effects, clears the SFX state.
        IFDEF PLY_AKM_MANAGE_SOUND_EFFECTS
dknr3 (void):          ld hl,0
                ld (PLY_AKM_Channel1_SoundEffectData),hl
                ld (PLY_AKM_Channel2_SoundEffectData),hl
                ld (PLY_AKM_Channel3_SoundEffectData),hl
        ENDIF ;PLY_AKM_MANAGE_SOUND_EFFECTS
        
        ;For ROM, generates the RET table.
        IFDEF PLY_AKM_Rom
                ld ix,PLY_AKM_RegistersForRom           ;Source.
                ld iy,PLY_AKM_Registers_RetTable        ;Destination.
                ld bc,PLY_AKM_SendPsgRegister
dknr3 (void):          ld de,4
PLY_AKM_InitRom_Loop:
                ld a,(ix)                 ;Gets the register.
                ld h,a
                inc ix
                and %00111111
                ld (iy + 0),a             ;Writes the register.
                ld (iy + 1),0             ;Value is 0 for now.
                ld a,h
                and %11000000
                jr nz,PLY_AKM_InitRom_Special
                ;Encodes the "normal" SendPsgRegister code address.
                ld (iy + 2),c
                ld (iy + 3),b
                add iy,de
                jr PLY_AKM_InitRom_Loop
PLY_AKM_InitRom_Special:
                IFDEF PLY_CFG_UseHardwareSounds         ;CONFIG SPECIFIC
                rl h
                jr c,PLY_AKM_InitRom_WriteEndCode
                ;Bit 6 must be set if we came here.
                ld bc,PLY_AKM_SendPsgRegisterR13
                ld (iy + 2),c
                ld (iy + 3),b
                ld bc,PLY_AKM_SendPsgRegisterAfterPop ;This one is a trick to send the register after R13 is managed.
                ld (iy + 4),c
                ld (iy + 5),b
                add iy,de               ;Only advance of 4, the code belows expects that.
                ENDIF ;PLY_CFG_UseHardwareSounds
                
PLY_AKM_InitRom_WriteEndCode:
                ld bc,PLY_AKM_SendPsgRegisterEnd
                ld (iy + 2),c
                ld (iy + 3),b
        ENDIF
        ret
        
        ;If ROM, the registers to send, IN THE ORDER they are declared in the ROM buffer!
        ;Bit 7 if ends (end DW to encode). Exclusive to bit 6.
        ;Bit 6 if R13/AfterPop the end DW to encode. Exclusive to bit 7.
        IFDEF PLY_AKM_Rom
PLY_AKM_RegistersForRom:
dkbs (void):
                db 8, 0, 1, 9, 2, 3, 10, 4, 5
                IFDEF PLY_AKM_USE_NoiseRegister          ;CONFIG SPECIFIC
                        db 6
                ENDIF
                IFNDEF PLY_CFG_UseHardwareSounds         ;CONFIG SPECIFIC
                        db 7 + 128
                ELSE
                        db 7, 11, 12 + 64     ;13 is NOT declared, special case.
                ENDIF
dkbe (void):
        ENDIF

;Addresses where to put the header data.
PLY_AKM_InitVars_Start:
dkps (void):
        dw PLY_AKM_NoteIndexTable + PLY_AKM_Offset1b
        dw PLY_AKM_NoteIndexTable + PLY_AKM_Offset1b + 1
        dw PLY_AKM_TrackIndex + PLY_AKM_Offset1b
        dw PLY_AKM_TrackIndex + PLY_AKM_Offset1b + 1
        dw PLY_AKM_Speed + PLY_AKM_Offset1b
        dw PLY_AKM_PrimaryInstrument + PLY_AKM_Offset1b
        dw PLY_AKM_SecondaryInstrument + PLY_AKM_Offset1b
        dw PLY_AKM_PrimaryWait + PLY_AKM_Offset1b
        dw PLY_AKM_SecondaryWait + PLY_AKM_Offset1b
        dw PLY_AKM_DefaultStartNoteInTracks + PLY_AKM_Offset1b
        dw PLY_AKM_DefaultStartInstrumentInTracks + PLY_AKM_Offset1b
        dw PLY_AKM_DefaultStartWaitInTracks + PLY_AKM_Offset1b
        dw PLY_AKM_FlagNoteAndEffectInCell + PLY_AKM_Offset1b
dkpe (void):
PLY_AKM_InitVars_End:


;Cuts the channels, stopping all sounds.
        if PLY_AKM_STOP_SOUNDS
PLY_AKM_StopDisarkGenerateExternalLabel:
PLY_AKM_Stop:
        ld (PLY_AKM_SaveSP + PLY_AKM_Offset1b),sp

        xor a
        ld (PLY_AKM_Track1_Volume),a
        ld (PLY_AKM_Track2_Volume),a
        ld (PLY_AKM_Track3_Volume),a
        IFDEF PLY_AKM_HARDWARE_MSX
                ld a,%10111111          ;On MSX, bit 7 must be 1, bit 6 0.
        ELSE
                ld a,%00111111          ;On CPC, bit 6 must be 0. Other platforms don't care.
        ENDIF
        ld (PLY_AKM_MixerRegister),a
        jp PLY_AKM_SendPsg
        endif ;PLY_AKM_STOP_SOUNDS



;Plays one frame of the song. It MUST have been initialized before.
;The stack is saved and restored, but is diverted, so watch out for the interruptions.
PLY_AKM_PlayDisarkGenerateExternalLabel:
PLY_AKM_Play:
        ld (PLY_AKM_SaveSP + PLY_AKM_Offset1b),sp

        ;Reads a new line?
        IFNDEF PLY_AKM_Rom
PLY_AKM_TickCounter: ld a,0
        inc a
PLY_AKM_Speed: cp 1                       ;Speed (>0).
        ELSE
        ld a,(PLY_AKM_Speed)
        ld b,a
        ld a,(PLY_AKM_TickCounter)
        inc a
        cp b
        ENDIF
        jp nz,PLY_AKM_TickCounterManaged

        ;A new line must be read. But have we reached the end of the Pattern?
        IFNDEF PLY_AKM_Rom
PLY_AKM_PatternRemainingHeight: ld a,0              ;Height. If 0, end of the pattern.
        ELSE
        ld a,(PLY_AKM_PatternRemainingHeight)
        ENDIF
        sub 1
        jr c,PLY_AKM_Linker
        ;Pattern not ended. No need to read the Linker.
        ld (PLY_AKM_PatternRemainingHeight + PLY_AKM_Offset1b),a
        jr PLY_AKM_ReadLine

        ;New pattern. Reads the Linker.
PLY_AKM_Linker:
        IFNDEF PLY_AKM_Rom
dknr3 (void):
PLY_AKM_TrackIndex: ld de,0              ;DE' points on the Track Index. Useful when new Tracks are found.
        ELSE
        ld de,(PLY_AKM_TrackIndex)
        ENDIF
        exx
        IFNDEF PLY_AKM_Rom
dknr3 (void):
PLY_AKM_PtLinker: ld hl,0
        ELSE
        ld hl,(PLY_AKM_PtLinker)
        ENDIF
PLY_AKM_LinkerPostPt:
        ;Resets the possible empty cell counter of each Track.
        xor a
        ld (PLY_AKM_Track1_WaitEmptyCell),a
        ld (PLY_AKM_Track2_WaitEmptyCell),a
        ld (PLY_AKM_Track3_WaitEmptyCell),a
        ;On new pattern, the escape note/instrument/wait values are set for each Tracks.
        IFNDEF PLY_AKM_Rom
PLY_AKM_DefaultStartNoteInTracks: ld a,0
        ELSE
        ld a,(PLY_AKM_DefaultStartNoteInTracks)
        ENDIF
        ld (PLY_AKM_Track1_EscapeNote),a
        ld (PLY_AKM_Track2_EscapeNote),a
        ld (PLY_AKM_Track3_EscapeNote),a
        IFNDEF PLY_AKM_Rom
PLY_AKM_DefaultStartInstrumentInTracks: ld a,0
        ELSE
        ld a,(PLY_AKM_DefaultStartInstrumentInTracks)
        ENDIF
        ld (PLY_AKM_Track1_EscapeInstrument),a
        ld (PLY_AKM_Track2_EscapeInstrument),a
        ld (PLY_AKM_Track3_EscapeInstrument),a
        IFNDEF PLY_AKM_Rom
PLY_AKM_DefaultStartWaitInTracks: ld a,0
        ELSE
        ld a,(PLY_AKM_DefaultStartWaitInTracks)
        ENDIF
        ld (PLY_AKM_Track1_EscapeWait),a
        ld (PLY_AKM_Track2_EscapeWait),a
        ld (PLY_AKM_Track3_EscapeWait),a

        ;Reads the state byte of the pattern.
        ld b,(hl)
        inc hl
        rr b             ;Speed change or end of song?
        jr nc,PLY_AKM_LinkerAfterSpeedChange
        ;Next byte is either the speed (>0) or an end of song marker.
        ld a,(hl)
        inc hl
                        ;If no speed used, it means "end of song" every time.
                        IFDEF PLY_CFG_UseSpeedTracks            ;CONFIG SPECIFIC        
        or a            ;0 if end of song, else speed.
        jr nz,PLY_AKM_LinkerSpeedChange
                        ENDIF ;PLY_CFG_UseSpeedTracks
        ;End of song.
        call player_end_song
        ld a,(hl)       ;Reads where to loop in the Linker.
        inc hl
        ld h,(hl)
        ld l,a
        jr PLY_AKM_LinkerPostPt
                        IFDEF PLY_CFG_UseSpeedTracks            ;CONFIG SPECIFIC        
PLY_AKM_LinkerSpeedChange:
        ;Speed change.
        ld (PLY_AKM_Speed + PLY_AKM_Offset1b),a
                        ENDIF ;PLY_CFG_UseSpeedTracks
PLY_AKM_LinkerAfterSpeedChange:

        ;New height?
        rr b
        jr nc,PLY_AKM_LinkerUsePreviousHeight
        ld a,(hl)
        inc hl
        ld (PLY_AKM_LinkerPreviousRemainingHeight + PLY_AKM_Offset1b),a
        jr PLY_AKM_LinkerSetRemainingHeight
        ;The same height is used. It was stored before.
PLY_AKM_LinkerUsePreviousHeight:
        IFNDEF PLY_AKM_Rom
PLY_AKM_LinkerPreviousRemainingHeight: ld a,0
        ELSE
        ld a,(PLY_AKM_LinkerPreviousRemainingHeight)
        ENDIF
PLY_AKM_LinkerSetRemainingHeight:
        ld (PLY_AKM_PatternRemainingHeight + PLY_AKM_Offset1b),a

        ;New Transposition and Track for channel 1?
        ld ix,PLY_AKM_Track1_Data
        call PLY_AKM_CheckTranspositionAndTrack
        ;New Transposition and Track for channel 2?
        ld ix,PLY_AKM_Track2_Data
        call PLY_AKM_CheckTranspositionAndTrack
        ;New Transposition and Track for channel 3?
        ld ix,PLY_AKM_Track3_Data
        call PLY_AKM_CheckTranspositionAndTrack
        
        ld (PLY_AKM_PtLinker + PLY_AKM_Offset1b),hl


;Reads the Tracks.
;---------------------------------
PLY_AKM_ReadLine:
        IFNDEF PLY_AKM_Rom
dknr3 (void):
PLY_AKM_PtInstruments: ld de,0
dknr3 (void):
PLY_AKM_NoteIndexTable: ld bc,0
        ELSE
        ld de,(PLY_AKM_PtInstruments)
        ld bc,(PLY_AKM_NoteIndexTable)
        ENDIF
        exx
                ld ix,PLY_AKM_Track1_Data
                call PLY_AKM_ReadTrack
                ld ix,PLY_AKM_Track2_Data
                call PLY_AKM_ReadTrack
                ld ix,PLY_AKM_Track3_Data
                call PLY_AKM_ReadTrack

                xor a
PLY_AKM_TickCounterManaged:
                ld (PLY_AKM_TickCounter + PLY_AKM_Offset1b),a



;Plays the sound stream.
;---------------------------------
                ld de,PLY_AKM_PeriodTable
        exx

        ld c,%11100000          ;Register 7, shifted of 2 to the left. Bits 2 and 5 will be possibly changed by each iteration.

        ld ix,PLY_AKM_Track1_Data
                        IFDEF PLY_CFG_UseEffects        ;CONFIG SPECIFIC
        call PLY_AKM_ManageEffects
                        ENDIF ;PLY_CFG_UseEffects
        ld iy,PLY_AKM_Track1_Registers
        call PLY_AKM_PlaySoundStream

        srl c                   ;Not RR, because we have to make sure the b6 is 0, else no more keyboard (on CPC)!
                                ;Also, on MSX, bit 6 must be 0.
        ld ix,PLY_AKM_Track2_Data
                        IFDEF PLY_CFG_UseEffects        ;CONFIG SPECIFIC
        call PLY_AKM_ManageEffects
                        ENDIF ;PLY_CFG_UseEffects
        ld iy,PLY_AKM_Track2_Registers
        call PLY_AKM_PlaySoundStream

        IFDEF PLY_AKM_HARDWARE_MSX
                scf             ;On MSX, bit 7 must be 1.
                rr c
        ELSE
                rr c            ;On other platforms, we don't care about b7.
        ENDIF
        ld ix,PLY_AKM_Track3_Data
                        IFDEF PLY_CFG_UseEffects        ;CONFIG SPECIFIC
        call PLY_AKM_ManageEffects
                        ENDIF ;PLY_CFG_UseEffects
        ld iy,PLY_AKM_Track3_Registers
        call PLY_AKM_PlaySoundStream

        ld a,c

;Plays the sound effects, if desired.
;-------------------------------------------
        IFDEF PLY_AKM_MANAGE_SOUND_EFFECTS
                        call PLY_AKM_PlaySoundEffectsStream
        ELSE
                        ld (PLY_AKM_MixerRegister),a
        ENDIF ;PLY_AKM_MANAGE_SOUND_EFFECTS



;Sends the values to the PSG.
;---------------------------------
PLY_AKM_SendPsg:
        ld sp,PLY_AKM_Registers_RetTable

        IFDEF PLY_AKM_HARDWARE_CPC
dknr3 (void):  ld bc,#f680
        ld a,#c0
dknr3 (void):  ld de,#f4f6
        out (c),a	;#f6c0          ;Madram's trick requires to start with this. out (c),b works, but will activate K7's relay! Not clean.
        ENDIF

        IFDEF PLY_AKM_HARDWARE_SPECTRUM_OR_PENTAGON
dknr3 (void):  ld de,#bfff
        ld c,#fd
        ENDIF

PLY_AKM_SendPsgRegister:
        pop hl          ;H = value, L = register.
PLY_AKM_SendPsgRegisterAfterPop:
        IFDEF PLY_AKM_HARDWARE_CPC
        ld b,d
        out (c),l       ;#f400 + register.
        ld b,e
        out (c),0       ;#f600
        ld b,d
        out (c),h       ;#f400 + value.
        ld b,e
        out (c),c       ;#f680
        out (c),a       ;#f6c0
        ENDIF

        IFDEF PLY_AKM_HARDWARE_SPECTRUM_OR_PENTAGON
        ld b,e
        out (c),l       ;#fffd + register.
        ld b,d
        out (c),h       ;#bffd + value
        ENDIF

        IFDEF PLY_AKM_HARDWARE_MSX
        ld a,l          ;Register.
        out (#a0),a
        ld a,h          ;Value.
        out (#a1),a
        ENDIF
        ret

                        IFDEF PLY_CFG_UseHardwareSounds         ;CONFIG SPECIFIC
PLY_AKM_SendPsgRegisterR13:

        ;Should the R13 be played? Yes only if different. No "force retrig" is managed by this player.
        IFNDEF PLY_AKM_Rom
PLY_AKM_SetReg13: ld a,0
PLY_AKM_SetReg13Old: cp 0
        ELSE
        ld a,(PLY_AKM_SetReg13Old)
        ld b,a
        ld a,(PLY_AKM_SetReg13)
        cp b
        ENDIF
        jr z,PLY_AKM_SendPsgRegisterEnd
        ;Different. R13 must be played. Updates the old R13 value.
        ld (PLY_AKM_SetReg13Old + PLY_AKM_Offset1b),a

        ld h,a
        ld l,13

        IFDEF PLY_AKM_HARDWARE_CPC
                ld a,#c0
        ENDIF

        ret                     ;Sends the 13th registers.
                        ENDIF ;PLY_CFG_UseHardwareSounds
PLY_AKM_SendPsgRegisterEnd:

        IFNDEF PLY_AKM_Rom
dknr3 (void):
PLY_AKM_SaveSP: ld sp,0
        ELSE
        ld sp,(PLY_AKM_SaveSP)
        ENDIF
        ret





;Shifts B to the right, if carry, a transposition is read.
;Shifts B to the right once again, if carry, a new Track is read (may be an index or a track offset).
;IN:    HL = where to read the data.
;       IX = points on the track data buffer.
;       DE'= Track index table
;       B  = flags.
;OUT:   B  = shifted of two.
;       HL = increased according to read data.
PLY_AKM_CheckTranspositionAndTrack:
        ;New transposition?
        rr b
                        IFDEF PLY_CFG_UseTranspositions         ;CONFIG SPECIFIC
        jr nc,PLY_AKM_CheckTranspositionAndTrack_AfterTransposition
        ;Transposition.
        ld a,(hl)
        ld (ix + PLY_AKM_Data_OffsetTransposition),a
        inc hl
PLY_AKM_CheckTranspositionAndTrack_AfterTransposition:
                        ENDIF ;PLY_CFG_UseTranspositions
        ;New Track?
        rr b
        jr nc,PLY_AKM_CheckTranspositionAndTrack_NoNewTrack
        ;New Track.
        ld a,(hl)
        inc hl
        ;Is it a reference?
        sla a
        jr nc,PLY_AKM_CheckTranspositionAndTrack_TrackOffset
        ;Reference.
        exx
                ld l,a          ;A is the track index * 2.
                ld h,0
                add hl,de       ;HL points on the track address.
                ld a,(hl)
                ld (ix + PLY_AKM_Data_OffsetPtStartTrack + 0),a
                ld (ix + PLY_AKM_Data_OffsetPtTrack + 0),a
                inc hl
                ld a,(hl)
                ld (ix + PLY_AKM_Data_OffsetPtStartTrack + 1),a
                ld (ix + PLY_AKM_Data_OffsetPtTrack + 1),a
        exx
        ret
PLY_AKM_CheckTranspositionAndTrack_TrackOffset:
        ;The Track is an offset. Counter the previous shift.
        rra             ;Carry was 0, so bit 7 is 0.
        ld d,a          ;D is the MSB of the offset.
        ld e,(hl)       ;Reads the LSB of the offset.
        inc hl
        
        ld c,l          ;Saves HL.
        ld a,h
        
        add hl,de       ;HL is now the Track (offset + $ (past offset));
        ld (ix + PLY_AKM_Data_OffsetPtStartTrack + 0),l
        ld (ix + PLY_AKM_Data_OffsetPtStartTrack + 1),h
        ld (ix + PLY_AKM_Data_OffsetPtTrack + 0),l
        ld (ix + PLY_AKM_Data_OffsetPtTrack + 1),h
        
        ld l,c          ;Retrieves HL.
        ld h,a
        ret
PLY_AKM_CheckTranspositionAndTrack_NoNewTrack:
        ;Copies the old Track inside the new Track pointer, as it evolves.
        ld a,(ix + PLY_AKM_Data_OffsetPtStartTrack + 0)
        ld (ix + PLY_AKM_Data_OffsetPtTrack + 0),a
        ld a,(ix + PLY_AKM_Data_OffsetPtStartTrack + 1)
        ld (ix + PLY_AKM_Data_OffsetPtTrack + 1),a
        ret





;Reads a Track.
;IN:    IX = Data block of the Track.
;       DE'= Instrument table. Do not modify!
;       BC'= Note index table. Do not modify!
PLY_AKM_ReadTrack:
        ;Are there any empty lines to wait?
        ld a,(ix + PLY_AKM_Data_OffsetWaitEmptyCell)
        sub 1
        jr c,PLY_AKM_RT_NoEmptyCell
        ;Wait!
        ld (ix + PLY_AKM_Data_OffsetWaitEmptyCell),a
        ret

PLY_AKM_RT_NoEmptyCell:
        ;Reads the Track pointer.
        ld l,(ix + PLY_AKM_Data_OffsetPtTrack + 0)
        ld h,(ix + PLY_AKM_Data_OffsetPtTrack + 1)
PLY_AKM_RT_GetDataByte:
        ld b,(hl)
        inc hl
        ;First, reads the note/effect flag.
        IFDEF PLY_AKM_Rom
        ld a,(PLY_AKM_FlagNoteAndEffectInCell)
        ld c,a
        ENDIF
        ld a,b
        and %1111       ;Keeps only the note/data.
        IFNDEF PLY_AKM_Rom
PLY_AKM_FlagNoteAndEffectInCell: cp 12          ;0-12 = note reference if no effects in the song, or 0-11 if there are effects in the song.
        ELSE
        cp c
        ENDIF
        jr c,PLY_AKM_RT_NoteReference
                        IFDEF PLY_CFG_UseEffects        ;CONFIG SPECIFIC
        sub 12                                  ;Can not be optimized with the code above, its value is automodified.
        jr z,PLY_AKM_RT_NoteAndEffects
        dec a
        jr z,PLY_AKM_RT_NoNoteMaybeEffects
                        ELSE
        sub 13
        jr z,PLY_AKM_RT_ReadWaitFlags          ;If no effects, directly check the wait flag.
                        ENDIF ;PLY_CFG_UseEffects
        dec a
        jr z,PLY_AKM_RT_NewEscapeNote
        ;15. Same escape note.
        ld a,(ix + PLY_AKM_Data_OffsetEscapeNote)
        jr PLY_AKM_RT_AfterNoteRead
        
PLY_AKM_RT_NewEscapeNote:
        ;Reads the escape note, and stores it, it may be reused by other cells.
        ld a,(hl)
        ld (ix + PLY_AKM_Data_OffsetEscapeNote),a
        inc hl
        jr PLY_AKM_RT_AfterNoteRead

                        IFDEF PLY_CFG_UseEffects        ;CONFIG SPECIFIC
PLY_AKM_RT_NoteAndEffects
        ;There is a "note and effects". This is a special case. A new data byte must be read, with the note and the normal flags.
        ;However, we use a "force effects" to mark the presence of effects.
        dec a     ;A is 0, give it any other value.
        ld (PLY_AKM_RT_ReadEffectsFlag + PLY_AKM_Offset1b),a
        jr PLY_AKM_RT_GetDataByte
        
PLY_AKM_RT_NoNoteMaybeEffects
        ;Reads flag "instrument" to know what to do. The flags are diverted to indicate whether there are effects.
        bit 4,b     ;Effects?
        jr z,PLY_AKM_RT_ReadWaitFlags  ;No effects. As there is no note, logically, there are no instrument to read, so simply reads the Wait value.
        ld a,b          ;B is not 0, so it works.
        ld (PLY_AKM_RT_ReadEffectsFlag + PLY_AKM_Offset1b),a
        jr PLY_AKM_RT_ReadWaitFlags
                        ENDIF ;PLY_CFG_UseEffects        
        
PLY_AKM_RT_NoteReference:
        ;A is the index of the note.
        exx
                ld l,a
                ld h,0
                add hl,bc
                ld a,(hl)
        exx

        ;A is the right note (0-127).
PLY_AKM_RT_AfterNoteRead:
        ;Adds the transposition.
                        IFDEF PLY_CFG_UseTranspositions         ;CONFIG SPECIFIC
        add a,(ix + PLY_AKM_Data_OffsetTransposition)
                        ENDIF ;PLY_CFG_UseTranspositions
        ld (ix + PLY_AKM_Data_OffsetBaseNote),a

        ;Reads the instruments flags.
        ;------------------
        ld a,b
        and %110000
        jr z,PLY_AKM_RT_SameEscapeInstrument
        cp %010000
        jr z,PLY_AKM_RT_PrimaryInstrument
        cp %100000
        jr z,PLY_AKM_RT_SecondaryInstrument
        ;New escape instrument. Reads and stores it, it may be reused by other cells.
        ld a,(hl)
        inc hl
        ld (ix + PLY_AKM_Data_OffsetEscapeInstrument),a
        jr PLY_AKM_RT_StoreCurrentInstrument

PLY_AKM_RT_SameEscapeInstrument:
        ;Use the latest escape instrument.
        ld a,(ix + PLY_AKM_Data_OffsetEscapeInstrument)
        jr PLY_AKM_RT_StoreCurrentInstrument

PLY_AKM_RT_SecondaryInstrument:
        ;Use the secondary instrument.
        IFNDEF PLY_AKM_Rom
PLY_AKM_SecondaryInstrument: ld a,0
        ELSE
        ld a,(PLY_AKM_SecondaryInstrument)
        ENDIF
        jr PLY_AKM_RT_StoreCurrentInstrument
        
PLY_AKM_RT_PrimaryInstrument:
        ;Use the primary instrument.
        IFNDEF PLY_AKM_Rom
PLY_AKM_PrimaryInstrument: ld a,0
        ELSE
        ld a,(PLY_AKM_PrimaryInstrument)
        ENDIF
        
PLY_AKM_RT_StoreCurrentInstrument:
        ;A is the instrument to play.
        exx
                ;Gets the address of the Instrument.
                add a,a         ;Only 127 instruments max.
                ld l,a
                ld h,0
                add hl,de       ;Adds to the Instrument Table.
                ld a,(hl)
                inc hl
                ld h,(hl)
                ld l,a
                ;Reads the header of the Instrument.
                ld a,(hl)       ;Speed.
                inc hl
                ld (ix + PLY_AKM_Data_OffsetInstrumentSpeed),a
                ;Stores the pointer on the data of the Instrument.
                ld (ix + PLY_AKM_Data_OffsetPtInstrument + 0),l
                ld (ix + PLY_AKM_Data_OffsetPtInstrument + 1),h
        exx
        xor a
        ;Resets the step on the Instrument.
        ld (ix + PLY_AKM_Data_OffsetInstrumentCurrentStep),a
        ;Resets the Track pitch.
                        IFDEF PLY_AKM_USE_EffectPitchUpDown        ;CONFIG SPECIFIC
        ld (ix + PLY_AKM_Data_OffsetIsPitchUpDownUsed),a
        ld (ix + PLY_AKM_Data_OffsetTrackPitchInteger + 0),a
        ld (ix + PLY_AKM_Data_OffsetTrackPitchInteger + 1),a
        ;ld (ix + PLY_AKM_Data_OffsetTrackPitchDecimal),a               ;Shouldn't be needed, the difference shouldn't be noticeable.
                        ENDIF ;PLY_AKM_USE_EffectPitchUpDown

        ;Resets the offset on Arpeggio and Pitch tables.
                        IFDEF PLY_CFG_UseEffect_ArpeggioTable        ;CONFIG SPECIFIC
        ld (ix + PLY_AKM_Data_OffsetPtArpeggioOffset),a
        ld (ix + PLY_AKM_Data_OffsetArpeggioCurrentStep),a
                                IFDEF PLY_CFG_UseEffect_ForceArpeggioSpeed        ;CONFIG SPECIFIC
        ld a,(ix + PLY_AKM_Data_OffsetArpeggioOriginalSpeed)            ;The arpeggio speed must be reset.
        ld (ix + PLY_AKM_Data_OffsetArpeggioCurrentSpeed),a
                                ENDIF ;PLY_CFG_UseEffect_ForceArpeggioSpeed
                        ENDIF ;PLY_CFG_UseEffect_ArpeggioTable

                        IFDEF PLY_CFG_UseEffect_PitchTable        ;CONFIG SPECIFIC
        ld (ix + PLY_AKM_Data_OffsetPtPitchOffset),a
        ld (ix + PLY_AKM_Data_OffsetPitchCurrentStep),a        
                                IFDEF PLY_CFG_UseEffect_ForcePitchTableSpeed        ;CONFIG SPECIFIC
        ld a,(ix + PLY_AKM_Data_OffsetPitchOriginalSpeed)               ;The pitch speed must be reset.
        ld (ix + PLY_AKM_Data_OffsetPitchCurrentSpeed),a
                                ENDIF ;PLY_CFG_UseEffect_ForcePitchTableSpeed
                        ENDIF ;PLY_CFG_UseEffect_PitchTable

        
        ;Reads the wait flags.
        ;----------------------
PLY_AKM_RT_ReadWaitFlags:
        ld a,b
        and %11000000
        jr z,PLY_AKM_RT_SameEscapeWait
        cp %01000000
        jr z,PLY_AKM_RT_PrimaryWait
        cp %10000000
        jr z,PLY_AKM_RT_SecondaryWait
        ;New escape wait. Reads and stores it, it may be reused by other cells.
        ld a,(hl)
        inc hl
        ld (ix + PLY_AKM_Data_OffsetEscapeWait),a
        jr PLY_AKM_RT_StoreCurrentWait
                
PLY_AKM_RT_SameEscapeWait:
        ;Use the latest escape wait.
        ld a,(ix + PLY_AKM_Data_OffsetEscapeWait)
        jr PLY_AKM_RT_StoreCurrentWait
        
PLY_AKM_RT_PrimaryWait:
        ;Use the primary wait.
        IFNDEF PLY_AKM_Rom
PLY_AKM_PrimaryWait: ld a,0
        ELSE
        ld a,(PLY_AKM_PrimaryWait)
        ENDIF
        jr PLY_AKM_RT_StoreCurrentWait

PLY_AKM_RT_SecondaryWait:
        ;Use the secondary wait.
        IFNDEF PLY_AKM_Rom
PLY_AKM_SecondaryWait: ld a,0
        ELSE
        ld a,(PLY_AKM_SecondaryWait)
        ENDIF

PLY_AKM_RT_StoreCurrentWait:
        ;A is the wait to store.
        ld (ix + PLY_AKM_Data_OffsetWaitEmptyCell),a

        ;--------------------
        ;Are there effects to read?
                        IFDEF PLY_CFG_UseEffects        ;CONFIG SPECIFIC
        IFNDEF PLY_AKM_Rom
PLY_AKM_RT_ReadEffectsFlag: ld a,0
        ELSE
        ld a,(PLY_AKM_RT_ReadEffectsFlag)
        ENDIF
        or a
        jr nz,PLY_AKM_RT_ReadEffects
PLY_AKM_RT_AfterEffects:
                        ENDIF ;PLY_CFG_UseEffects
        ;No effects, or after they have been managed.
        ;Saves the new pointer on the Track.
        ld (ix + PLY_AKM_Data_OffsetPtTrack + 0),l
        ld (ix + PLY_AKM_Data_OffsetPtTrack + 1),h
        ret
                        IFDEF PLY_CFG_UseEffects        ;CONFIG SPECIFIC
PLY_AKM_RT_ReadEffects:
        ;Resets the effect presence flag.
        xor a
        ld (PLY_AKM_RT_ReadEffectsFlag + PLY_AKM_Offset1b),a
        
PLY_AKM_RT_ReadEffect:
        ld iy,PLY_AKM_EffectTable
        ;Reads effect number and possible data. All effect must jump to PLY_AKM_RT_ReadEffect_Return when finished.
        ld b,(hl)
        ld a,b
        inc hl
        
        and %1110
        ld e,a
        ld d,0
        add iy,de
        
        ;As a convenience, puts the effect nibble "to the right", for direct use.
        ld a,b
        rra
        rra
        rra
        rra
        and %1111               ;This sets the carry flag, useful for the effects code.
        ;Executes the effect code.
        jp (iy)
PLY_AKM_RT_ReadEffect_Return:
        ;More effects?
        bit 0,b
        jr nz,PLY_AKM_RT_ReadEffect
        jr PLY_AKM_RT_AfterEffects

PLY_AKM_RT_WaitLong:
        ;A 8-bit byte is encoded just after.
        ld a,(hl)
        inc hl
        ld (ix + PLY_AKM_Data_OffsetWaitEmptyCell),a
        jr PLY_AKM_RT_CellRead
PLY_AKM_RT_WaitShort:
        ;Only a 2-bit value is encoded.
        ld a,b
        rlca                     ;Transfers the bit 7/6 to 1/0. Thanks Hicks for the RCLA trick!
        rlca
        and %11
        ld (ix + PLY_AKM_Data_OffsetWaitEmptyCell),a
        ;jr PLY_AKM_RT_CellRead
;Jumped to after the Cell has been read.
;IN:    HL = new value of the Track pointer. Must point after the read Cell.
PLY_AKM_RT_CellRead:
        ld (ix + PLY_AKM_Data_OffsetPtTrack + 0),l
        ld (ix + PLY_AKM_Data_OffsetPtTrack + 1),h
        ret


;Manages the effects, if any. For the activated effects, modifies the internal data for the Track which data block is given.
;IN:    IX = data block of the Track.
;OUT:   IX, IY = unmodified.
;       C must NOT be modified!
;       DE' must NOT be modified!
PLY_AKM_ManageEffects:
                                IFDEF PLY_AKM_USE_EffectPitchUpDown        ;CONFIG SPECIFIC
        ;Pitch up/down used?
        ld a,(ix + PLY_AKM_Data_OffsetIsPitchUpDownUsed)
        or a
        jr z,PLY_AKM_ME_PitchUpDownFinished

        ;Adds the LSB of integer part and decimal part, using one 16 bits operation.
        ld l,(ix + PLY_AKM_Data_OffsetTrackPitchDecimal)
        ld h,(ix + PLY_AKM_Data_OffsetTrackPitchInteger + 0)

        ld e,(ix + PLY_AKM_Data_OffsetTrackPitchSpeed + 0)
        ld d,(ix + PLY_AKM_Data_OffsetTrackPitchSpeed + 1)

        ld a,(ix + PLY_AKM_Data_OffsetTrackPitchInteger + 1)

        ;Negative pitch?
        bit 7,d
        jr nz,PLY_AKM_ME_PitchUpDown_NegativeSpeed

PLY_AKM_ME_PitchUpDown_PositiveSpeed:
        ;Positive speed. Adds it to the LSB of the integer part, and decimal part.
        add hl,de

        ;Carry? Transmits it to the MSB of the integer part.
        adc 0
        jr PLY_AKM_ME_PitchUpDown_Save
PLY_AKM_ME_PitchUpDown_NegativeSpeed:
        ;Negative speed. Resets the sign bit. The encoded pitch IS positive.
        ;Subtracts it to the LSB of the integer part, and decimal part.
        res 7,d

        or a
        sbc hl,de

        ;Carry? Transmits it to the MSB of the integer part.
        sbc 0

PLY_AKM_ME_PitchUpDown_Save:
        ld (ix + PLY_AKM_Data_OffsetTrackPitchInteger + 1),a

        ld (ix + PLY_AKM_Data_OffsetTrackPitchDecimal),l
        ld (ix + PLY_AKM_Data_OffsetTrackPitchInteger + 0),h

PLY_AKM_ME_PitchUpDownFinished:
                                ENDIF ;PLY_AKM_USE_EffectPitchUpDown



        ;Manages the Arpeggio Table effect, if any.
        ;------------------------------------------
                                IFDEF PLY_CFG_UseEffect_ArpeggioTable        ;CONFIG SPECIFIC
        ld a,(ix + PLY_AKM_Data_OffsetIsArpeggioTableUsed)
        or a
        jr z,PLY_AKM_ME_ArpeggioTableFinished

        ;Plays the arpeggio current note. It is suppose to be correct (not a loop).
        ;Plays it in any case, in order to manage some corner case with Force Arpeggio Speed.
        ld e,(ix + PLY_AKM_Data_OffsetPtArpeggioTable + 0)
        ld d,(ix + PLY_AKM_Data_OffsetPtArpeggioTable + 1)
        ld l,(ix + PLY_AKM_Data_OffsetPtArpeggioOffset)
        ld h,0
        add hl,de
        ld a,(hl)       ;Gets the Arpeggio value (b1-b7).
        sra a           ;Carry is 0, because the ADD above surely didn't overflow.
        ld (ix + PLY_AKM_Data_OffsetCurrentArpeggioValue),a

        ;Moves forward, if the speed has been reached.
        ;Has the speed been reached?
        ld a,(ix + PLY_AKM_Data_OffsetArpeggioCurrentStep)
                                IFDEF PLY_CFG_UseEffect_ForceArpeggioSpeed        ;CONFIG SPECIFIC
        cp (ix + PLY_AKM_Data_OffsetArpeggioCurrentSpeed)
                                ELSE
        cp (ix + PLY_AKM_Data_OffsetArpeggioOriginalSpeed)
                                ENDIF ;PLY_CFG_UseEffect_ForceArpeggioSpeed
        jr c,PLY_AKM_ME_ArpeggioTable_SpeedNotReached
        ;Resets the speed. Reads the next Arpeggio value.
        ld (ix + PLY_AKM_Data_OffsetArpeggioCurrentStep),0
        
        ;Advances in the Arpeggio.
        inc (ix + PLY_AKM_Data_OffsetPtArpeggioOffset)
        inc hl          ;HL points on the next value. No need to add to the base offset like before, we have it.
        ld a,(hl)
        ;End of the Arpeggio?
        rra             ;Carry is 0.
        jr nc,PLY_AKM_ME_ArpeggioTableFinished
        ;End of the Arpeggio. The loop offset is now in A.
        ld l,a
        ld (ix + PLY_AKM_Data_OffsetPtArpeggioOffset),a
        jr PLY_AKM_ME_ArpeggioTableFinished
        
PLY_AKM_ME_ArpeggioTable_SpeedNotReached:
        inc a
        ld (ix + PLY_AKM_Data_OffsetArpeggioCurrentStep),a

PLY_AKM_ME_ArpeggioTableFinished:
                                ENDIF ;PLY_CFG_UseEffect_ArpeggioTable



        ;Manages the Pitch Table effect, if any.
        ;------------------------------------------
                                IFDEF PLY_CFG_UseEffect_PitchTable        ;CONFIG SPECIFIC
        ld a,(ix + PLY_AKM_Data_OffsetIsPitchTableUsed)
        or a
        ret z

        ;Plays the Pitch Table current note. It is suppose to be correct (not a loop).
        ;Plays it in any case, in order to manage some corner case with Force Pitch Speed.
        ;Reads the Pitch Table. Adds the Pitch base address to an offset.
        ld l,(ix + PLY_AKM_Data_OffsetPtPitchTable + 0)
        ld h,(ix + PLY_AKM_Data_OffsetPtPitchTable + 1)
        ld e,(ix + PLY_AKM_Data_OffsetPtPitchOffset)
        ld d,0
        add hl,de
        ld a,(hl)       ;Gets the Pitch value (b1-b7).
        sra a
        ;A = pitch note. It is converted to 16 bits.
        ;D is already 0.
        jp p,PLY_AKM_ME_PitchTableEndNotReached_Positive
        dec d
PLY_AKM_ME_PitchTableEndNotReached_Positive:
        ld (ix + PLY_AKM_Data_OffsetCurrentPitchTableValue + 0),a
        ld (ix + PLY_AKM_Data_OffsetCurrentPitchTableValue + 1),d
        
        ;Moves forward, if the speed has been reached.
        ;Has the speed been reached?
        ld a,(ix + PLY_AKM_Data_OffsetPitchCurrentStep)
                        IFDEF PLY_CFG_UseEffect_ForcePitchTableSpeed        ;CONFIG SPECIFIC
        cp (ix + PLY_AKM_Data_OffsetPitchCurrentSpeed)
                        ELSE
        cp (ix + PLY_AKM_Data_OffsetPitchOriginalSpeed)
                        ENDIF ;PLY_CFG_UseEffect_ForcePitchTableSpeed
        jr c,PLY_AKM_ME_PitchTable_SpeedNotReached
        ;Resets the speed, then reads the next Pitch value.
        ld (ix + PLY_AKM_Data_OffsetPitchCurrentStep),0
        
        ;Advances in the Pitch.
        inc (ix + PLY_AKM_Data_OffsetPtPitchOffset)
        inc hl          ;HL points on the next value. No need to add to the base offset like before, we have it.
        ld a,(hl)
        ;End of the Pitch?
        rra             ;Carry is 0.
        ret nc
        ;End of the Pitch. The loop offset is now in A.
        ld l,a
        ld (ix + PLY_AKM_Data_OffsetPtPitchOffset),a
        ret

PLY_AKM_ME_PitchTable_SpeedNotReached:
        inc a
        ld (ix + PLY_AKM_Data_OffsetPitchCurrentStep),a
                                ENDIF ;PLY_CFG_UseEffect_PitchTable
        ret
                        ENDIF ;PLY_CFG_UseEffects
        
        


;---------------------------------------------------------------------
;Sound stream.
;---------------------------------------------------------------------

;Plays the sound stream, filling the PSG registers table (but not playing it).
;The Instrument pointer must be updated as it evolves inside the Instrument.
;IN:    IX = Data block of the Track.
;       IY = Points at the beginning of the register structure related to the channel.
;       C = R7. Only bit 2 (sound) must be *set* to cut the sound if needed, and bit 5 (noise) must be *reset* if there is noise.
;       DE' = Period table. Must not be modified.
PLY_AKM_PlaySoundStream:
        ;Gets the pointer on the Instrument, from its base address and the offset.
        ld l,(ix + PLY_AKM_Data_OffsetPtInstrument + 0)
        ld h,(ix + PLY_AKM_Data_OffsetPtInstrument + 1)

        ;Reads the first byte of the cell of the Instrument. What type?
PLY_AKM_PSS_ReadFirstByte:
        ld a,(hl)
        ld b,a
        inc hl
        rra
        jr c,PLY_AKM_PSS_SoftOrSoftAndHard

        ;NoSoftNoHard or SoftwareToHardware
        rra
                        IFDEF PLY_CFG_UseHardwareSounds         ;CONFIG SPECIFIC
        jr c,PLY_AKM_PSS_SoftwareToHardware
                        ENDIF ;PLY_CFG_UseHardwareSounds

        ;No software no hardware, or end of sound (loop)!
        ;End of sound?
        rra
        jr nc,PLY_AKM_PSS_NSNH_NotEndOfSound
        ;The sound loops/ends. Where?
        ld a,(hl)
        inc hl
        ld h,(hl)
        ld l,a
        ;As a sound always has at least one cell, we should safely be able to read its bytes without storing the instrument pointer.
        ;However, we do it anyway to remove the overhead of the Speed management: if looping, the same last line will be read,
        ;if several channels do so, it will be costly. So...
        ld (ix + PLY_AKM_Data_OffsetPtInstrument + 0),l
        ld (ix + PLY_AKM_Data_OffsetPtInstrument + 1),h
        jr PLY_AKM_PSS_ReadFirstByte

PLY_AKM_PSS_NSNH_NotEndOfSound:
        ;No software, no hardware.
        ;-------------------------
        ;Stops the sound.
        set 2,c

        ;Volume. A now contains the volume on b0-3.
                        IFDEF PLY_CFG_UseEffect_SetVolume        ;CONFIG SPECIFIC
        call PLY_AKM_PSS_Shared_AdjustVolume
                        ELSE
        and %1111
                        ENDIF ;PLY_CFG_UseEffect_SetVolume
        ld (iy + PLY_AKM_Registers_OffsetVolume),a

        ;Read noise?
        rl b
                        IFDEF PLY_CFG_NoSoftNoHard_Noise        ;CONFIG SPECIFIC
        call c,PLY_AKM_PSS_ReadNoise
                        ENDIF ;PLY_CFG_NoSoftNoHard_Noise
        jr PLY_AKM_PSS_Shared_StoreInstrumentPointer

        ;Software sound, or Software and Hardware?
PLY_AKM_PSS_SoftOrSoftAndHard:
        rra
                        IFDEF PLY_CFG_UseHardwareSounds         ;CONFIG SPECIFIC
        jr c,PLY_AKM_PSS_SoftAndHard
                        ENDIF ;PLY_CFG_UseHardwareSounds

        ;Software sound.
        ;-----------------
        ;A is the volume. Already shifted twice, so it can be used directly.
                        IFDEF PLY_CFG_UseEffect_SetVolume        ;CONFIG SPECIFIC
        call PLY_AKM_PSS_Shared_AdjustVolume
                        ELSE
        and %1111
                        ENDIF ;PLY_CFG_UseEffect_SetVolume
        ld (iy + PLY_AKM_Registers_OffsetVolume),a

        ;Arp and/or noise?
        ld d,0          ;Default arpeggio.
        rl b
        jr nc,PLY_AKM_PSS_S_AfterArpAndOrNoise
        ld a,(hl)
        inc hl
        ;Noise?
        sra a
        ;A is now the signed Arpeggio. It must be kept.
        ld d,a
        ;Now takes care of the noise, if there is a Carry.
                        IFDEF PLY_CFG_SoftOnly_Noise          ;CONFIG SPECIFIC
        call c,PLY_AKM_PSS_ReadNoise
                        ENDIF ;PLY_CFG_SoftOnly_Noise
PLY_AKM_PSS_S_AfterArpAndOrNoise:

        ld a,d          ;Gets the instrument arpeggio, if any.
        call PLY_AKM_CalculatePeriodForBaseNote

        ;Read pitch?
        rl b
                        IFDEF PLY_CFG_SoftOnly_SoftwarePitch    ;CONFIG SPECIFIC
        call c,PLY_AKM_ReadPitchAndAddToPeriod
                        ENDIF ;PLY_CFG_SoftOnly_SoftwarePitch

        ;Stores the new period of this channel.
        exx
                ld (iy + PLY_AKM_Registers_OffsetSoftwarePeriodLSB),l
                ld (iy + PLY_AKM_Registers_OffsetSoftwarePeriodMSB),h
        exx

        ;The code below is shared!
        ;Stores the new instrument pointer, if Speed allows it.
        ;--------------------------------------------------
PLY_AKM_PSS_Shared_StoreInstrumentPointer:
        ;Checks the Instrument speed, and only stores the Instrument new pointer if the speed is reached.
        ld a,(ix + PLY_AKM_Data_OffsetInstrumentCurrentStep)
        cp (ix + PLY_AKM_Data_OffsetInstrumentSpeed)
        jr nc,PLY_AKM_PSS_S_SpeedReached
        ;Increases the current step.
        inc (ix + PLY_AKM_Data_OffsetInstrumentCurrentStep)
        ret
PLY_AKM_PSS_S_SpeedReached:
        ;Stores the Instrument new pointer, resets the speed counter.
        ld (ix + PLY_AKM_Data_OffsetPtInstrument + 0),l
        ld (ix + PLY_AKM_Data_OffsetPtInstrument + 1),h
        ld (ix + PLY_AKM_Data_OffsetInstrumentCurrentStep),0
        ret


                        IFDEF PLY_CFG_UseHardwareSounds         ;CONFIG SPECIFIC

        ;Software and Hardware.
        ;----------------------------
PLY_AKM_PSS_SoftAndHard:
        ;Reads the envelope bit, the possible pitch, and sets the software period accordingly.
        call PLY_AKM_PSS_Shared_ReadEnvBitPitchArp_SoftPeriod_HardVol_HardEnv
        ;Reads the hardware period.
        ld a,(hl)
        ld (PLY_AKM_Reg11),a
        inc hl
        ld a,(hl)
        ld (PLY_AKM_Reg12),a
        inc hl

        jr PLY_AKM_PSS_Shared_StoreInstrumentPointer


        ;Software to Hardware.
        ;-------------------------
PLY_AKM_PSS_SoftwareToHardware:
        call PLY_AKM_PSS_Shared_ReadEnvBitPitchArp_SoftPeriod_HardVol_HardEnv

        ;Now we can calculate the hardware period thanks to the ratio (contray to LW, it is NOT inverted, we can use it as-is).
        ld a,b
        rlca
        rlca
        rlca
        rlca
        and %111
        exx
                jr z,PLY_AKM_PSS_STH_RatioEnd
PLY_AKM_PSS_STH_RatioLoop:
                srl h
                rr l
                dec a 
                jr nz,PLY_AKM_PSS_STH_RatioLoop
                ;If carry, rounds the period.
                jr nc,PLY_AKM_PSS_STH_RatioEnd
                inc hl
PLY_AKM_PSS_STH_RatioEnd:
                ld a,l
                ld (PLY_AKM_Reg11),a
                ld a,h
                ld (PLY_AKM_Reg12),a
        exx

        jr PLY_AKM_PSS_Shared_StoreInstrumentPointer

;A shared code for hardware sound.
;Reads the envelope bit in bit 1, arpeggio in bit 7 pitch in bit 2 from A. If pitch present, adds it to BC'.
;Converts the note to period, adds the instrument pitch, sets the software period of the channel.
;Also sets the hardware volume, and sets the hardware curve.
PLY_AKM_PSS_Shared_ReadEnvBitPitchArp_SoftPeriod_HardVol_HardEnv:
        ;Envelope bit? R13 = 8 + 2 * (envelope bit?). Allows to have hardware envelope to 8 or 0xa.
        ;Shifted by 2 to the right, bit 1 is now envelope bit, which is perfect for us.
        and %10
        add a,8
        ld (PLY_AKM_SetReg13 + PLY_AKM_Offset1b),a

        ;Volume to 16 to trigger the hardware envelope.
        ld (iy + PLY_AKM_Registers_OffsetVolume),16

        ;Arpeggio?
        xor a                   ;Default arpeggio.
                        IFDEF PLY_AKM_ArpeggioInHardwareInstrument  ;CONFIG SPECIFIC
        bit 7,b                 ;Not shifted yet.
        jr z,PLY_AKM_PSS_Shared_REnvBAP_AfterArpeggio
        ;Reads the Arpeggio.
        ld a,(hl)
        inc hl
PLY_AKM_PSS_Shared_REnvBAP_AfterArpeggio:
                        ENDIF ;PLY_AKM_ArpeggioInHardwareInstrument
        ;Calculates the software period.
        call PLY_AKM_CalculatePeriodForBaseNote

        ;Pitch?
                        IFDEF PLY_AKM_PitchInHardwareInstrument  ;CONFIG SPECIFIC
        bit 2,b         ;Not shifted yet.
        call nz,PLY_AKM_ReadPitchAndAddToPeriod
                        ENDIF ;PLY_AKM_PitchInHardwareInstrument

        ;Stores the new period of this channel.
        exx
                ld (iy + PLY_AKM_Registers_OffsetSoftwarePeriodLSB),l
                ld (iy + PLY_AKM_Registers_OffsetSoftwarePeriodMSB),h
        exx
        ret

                        ENDIF ;PLY_CFG_UseHardwareSounds

                        IFDEF PLY_CFG_UseEffect_SetVolume        ;CONFIG SPECIFIC
;Decreases the given volume (encoded in possibly more then 4 bits). If <0, forced to 0.
;IN:    A = volume, not ANDed.
;OUT:   A = new volume.
PLY_AKM_PSS_Shared_AdjustVolume:
        and %1111
        sub (ix + PLY_AKM_Data_OffsetTrackInvertedVolume)
        ret nc
        xor a
        ret
                        ENDIF ;PLY_CFG_UseEffect_SetVolume

;Reads and stores the noise pointed by HL, opens the noise channel.
;IN:    HL = instrument data where the noise is.
;OUT:   HL = HL++.
;MOD:   A.
                IFDEF PLY_AKM_USE_Noise          ;CONFIG SPECIFIC
PLY_AKM_PSS_ReadNoise:
        ld a,(hl)
        inc hl
        ld (PLY_AKM_NoiseRegister),a
        res 5,c                 ;Opens the noise channel.
        ret
                ENDIF ;PLY_AKM_USE_Noise
                
;Calculates the period according to the base note and put it in BC'. Used by both software and hardware codes.
;IN:    DE' = period table.
;       A = instrument arpeggio (0 if not used).
;OUT:   HL' = period.
;MOD:   A
PLY_AKM_CalculatePeriodForBaseNote:
        ;Gets the period from the current note.
        exx
                ld h,0
                add a,(ix + PLY_AKM_Data_OffsetBaseNote)                         ;Adds the instrument Arp to the base note (including the transposition).
                                IFDEF PLY_CFG_UseEffect_ArpeggioTable            ;CONFIG SPECIFIC
                add (ix + PLY_AKM_Data_OffsetCurrentArpeggioValue)               ;Adds the Arpeggio Table effect.
                                ENDIF ;PLY_CFG_UseEffect_ArpeggioTable

                ;Finds the period from a single line of octave look-up table. This is slow...
                ;IN:    DE = PeriodTable.
                ;       A = note (>=0).
                ;OUT:   HL = period.
                ;       DE unmodified.
                ;       BC modified.

                ;Finds the octave.
dknr3 (void):   ld bc,255 * 256 + 12            ;B = Octave (>=0). Will be increased just below.
PLY_AKM_FindOctave_Loop:
                inc b           ;Next octave.
                sub c
                jr nc,PLY_AKM_FindOctave_Loop
                add a,c         ;Compensates the first iteration that may not have been useful.
        
                ;A = note inside the octave. Gets the period for the note, for the lowest octave.
                add a,a
                ld l,a
                ld h,0
                add hl,de       ;Points on the period on the lowest octave.
                ld a,(hl)
                inc hl
                ld h,(hl)       ;HL is the period on the lowest octave.
                ld l,a
                ;Divides the period as long as we haven't reached the octave.
                ld a,b
                or a
                jr z,PLY_AKM_FindOctave_OctaveShiftLoop_Finished
PLY_AKM_FindOctave_OctaveShiftLoop:
                srl h
                rr l
                djnz PLY_AKM_FindOctave_OctaveShiftLoop          ;Fortunately, does not modify the carry, used below.
PLY_AKM_FindOctave_OctaveShiftLoop_Finished:
                ;Rounds the period at the last iteration.
                jr nc,PLY_AKM_FindOctave_Finished
                inc hl
PLY_AKM_FindOctave_Finished:

                ;Adds the Pitch Table value, if used.
                        IFDEF PLY_CFG_UseEffect_PitchTable        ;CONFIG SPECIFIC
                ld a,(ix + PLY_AKM_Data_OffsetIsPitchTableUsed)
                or a
                jr z,PLY_AKM_CalculatePeriodForBaseNote_NoPitchTable
                ld c,(ix + PLY_AKM_Data_OffsetCurrentPitchTableValue + 0)
                ld b,(ix + PLY_AKM_Data_OffsetCurrentPitchTableValue + 1)
                add hl,bc
PLY_AKM_CalculatePeriodForBaseNote_NoPitchTable:
                        ENDIF ;PLY_CFG_UseEffect_PitchTable
                ;Adds the Track Pitch.
                        IFDEF PLY_AKM_USE_EffectPitchUpDown        ;CONFIG SPECIFIC
                ld c,(ix + PLY_AKM_Data_OffsetTrackPitchInteger + 0)
                ld b,(ix + PLY_AKM_Data_OffsetTrackPitchInteger + 1)
                add hl,bc
                        ENDIF ;PLY_AKM_USE_EffectPitchUpDown
        exx
        ret

                        IFDEF PLY_AKM_PitchInInstrument  ;CONFIG SPECIFIC
;Reads the pitch in the Instruments (16 bits) and adds it to HL', which should contain the software period.
;IN:    HL = points on the pitch value.
;OUT:   HL = points after the pitch.
;MOD:   A, BC', HL' updated.
PLY_AKM_ReadPitchAndAddToPeriod:
        ;Reads 2 * 8 bits for the pitch. Slow...
        ld a,(hl)
        inc hl
        exx
                ld c,a                  ;Adds the read pitch to the note period.
        exx
        ld a,(hl)
        inc hl
        exx
                ld b,a
                add hl,bc
        exx
        ret
                        ENDIF ;PLY_AKM_PitchInInstrument







;---------------------------------------------------------------------
;Effect management.
;---------------------------------------------------------------------

                        IFDEF PLY_CFG_UseEffects        ;CONFIG SPECIFIC
                        
;IN:    HL = points after the first byte.
;       A = data of the first byte on bits 0-3, the other bits are 0.
;       Carry = 0.
;       Z flag = 1 if the data is 0.
;       DE'= Instrument Table (not useful here). Do not modify!
;       IX = data block of the Track.
;       B = Do not modify!
;OUT:   HL = points after the data of the effect (maybe nothing to do).
;       Each effect must jump to PLY_AKM_RT_ReadEffect_Return.

                                IFDEF PLY_CFG_UseEffect_Reset           ;CONFIG SPECIFIC.
;Clears all the effects (volume, pitch table, arpeggio table).
PLY_AKM_EffectResetWithVolume:
        ;Inverted volume.
                                        IFDEF PLY_CFG_UseEffect_SetVolume        ;CONFIG SPECIFIC
        ld (ix + PLY_AKM_Data_OffsetTrackInvertedVolume),a
                                        ENDIF ;PLY_CFG_UseEffect_SetVolume
        xor a
        ;The inverted volume is managed above, so don't change it.
                                        IFDEF PLY_AKM_USE_EffectPitchUpDown        ;CONFIG SPECIFIC
        ld (ix + PLY_AKM_Data_OffsetIsPitchUpDownUsed),a
                                        ENDIF ;PLY_AKM_USE_EffectPitchUpDown
                                        IFDEF PLY_CFG_UseEffect_ArpeggioTable        ;CONFIG SPECIFIC
        ld (ix + PLY_AKM_Data_OffsetIsArpeggioTableUsed),a
        ld (ix + PLY_AKM_Data_OffsetCurrentArpeggioValue),a      ;Contrary to the Pitch, the value must be reset.
                                        ENDIF ;PLY_CFG_UseEffect_ArpeggioTable
                                        IFDEF PLY_CFG_UseEffect_PitchTable        ;CONFIG SPECIFIC
        ld (ix + PLY_AKM_Data_OffsetIsPitchTableUsed),a
                                        ENDIF ;PLY_CFG_UseEffect_PitchTable
        jp PLY_AKM_RT_ReadEffect_Return
                                ENDIF ;PLY_CFG_UseEffect_Reset


;Changes the volume.
                                IFDEF PLY_CFG_UseEffect_SetVolume        ;CONFIG SPECIFIC
PLY_AKM_EffectVolume:
        ld (ix + PLY_AKM_Data_OffsetTrackInvertedVolume),a
        jp PLY_AKM_RT_ReadEffect_Return
                                ENDIF ;PLY_CFG_UseEffect_SetVolume

                                IFDEF PLY_CFG_UseEffect_ForceInstrumentSpeed        ;CONFIG SPECIFIC
;Forces the speed of the Instrument. The current step is NOT changed.
PLY_AKM_EffectForceInstrumentSpeed:
        call PLY_AKM_EffectReadIfEscape         ;Makes sure the data is 0-14, else 15 means: read the next escape value.
        ld (ix + PLY_AKM_Data_OffsetInstrumentSpeed),a
        
        jp PLY_AKM_RT_ReadEffect_Return
                                ENDIF ;PLY_CFG_UseEffect_ForceInstrumentSpeed

                                IFDEF PLY_CFG_UseEffect_ForcePitchTableSpeed        ;CONFIG SPECIFIC
;Forces the speed of the Pitch. The current step is NOT changed.
PLY_AKM_EffectForcePitchSpeed:
        call PLY_AKM_EffectReadIfEscape         ;Makes sure the data is 0-14, else 15 means: read the next escape value.
        ld (ix + PLY_AKM_Data_OffsetPitchCurrentSpeed),a
        ;ld (ix + PLY_AKM_Data_OffsetPitchCurrentStep),a                ;No need to force next note of the Arpeggio. Faster, and more compliant with the C++ player.
        
        jp PLY_AKM_RT_ReadEffect_Return
                                ENDIF ;PLY_CFG_UseEffect_ForcePitchTableSpeed
        
                                IFDEF PLY_CFG_UseEffect_ForceArpeggioSpeed        ;CONFIG SPECIFIC
;Forces the speed of the Arpeggio. The current step is NOT changed.
PLY_AKM_EffectForceArpeggioSpeed:
        call PLY_AKM_EffectReadIfEscape         ;Makes sure the data is 0-14, else 15 means: read the next escape value.
        ld (ix + PLY_AKM_Data_OffsetArpeggioCurrentSpeed),a
        ;ld (ix + PLY_AKM_Data_OffsetArpeggioCurrentStep),a             ;No need to force next note of the Arpeggio. Faster, and more compliant with the C++ player.
        
        jp PLY_AKM_RT_ReadEffect_Return
                                ENDIF ;PLY_CFG_UseEffect_ForceArpeggioSpeed


;Effect table. Each entry jumps to an effect management code.
;Put after the code above so that the JR are within bound.
PLY_AKM_EffectTable:
                                IFDEF PLY_CFG_UseEffect_Reset           ;CONFIG SPECIFIC.
        jr PLY_AKM_EffectResetWithVolume                         ;000
                                ELSE
                                jr $
                                ENDIF ;PLY_CFG_UseEffect_Reset
                                
                                IFDEF PLY_CFG_UseEffect_SetVolume        ;CONFIG SPECIFIC
        jr PLY_AKM_EffectVolume                                  ;001
                                ELSE
                                jr $
                                ENDIF ;PLY_CFG_UseEffect_SetVolume
        
                                IFDEF PLY_AKM_USE_EffectPitchUpDown        ;CONFIG SPECIFIC
        jr PLY_AKM_EffectPitchUpDown                             ;010
                                ELSE
                                jr $
                                ENDIF ;PLY_AKM_USE_EffectPitchUpDown

                                IFDEF PLY_CFG_UseEffect_ArpeggioTable        ;CONFIG SPECIFIC
        jr PLY_AKM_EffectArpeggioTable                           ;011
                                ELSE
                                jr $
                                ENDIF ;PLY_CFG_UseEffect_ArpeggioTable
                               
                                IFDEF PLY_CFG_UseEffect_PitchTable        ;CONFIG SPECIFIC
        jr PLY_AKM_EffectPitchTable                              ;100
                                ELSE
                                jr $
                                ENDIF ;PLY_CFG_UseEffect_PitchTable
        
                                IFDEF PLY_CFG_UseEffect_ForceInstrumentSpeed        ;CONFIG SPECIFIC
        jr PLY_AKM_EffectForceInstrumentSpeed                    ;101
                                ELSE
                                jr $
                                ENDIF ;PLY_CFG_UseEffect_ForceInstrumentSpeed
        
                                IFDEF PLY_CFG_UseEffect_ForceArpeggioSpeed        ;CONFIG SPECIFIC
        jr PLY_AKM_EffectForceArpeggioSpeed                      ;110
                                ELSE
                                jr $
                                ENDIF ;PLY_CFG_UseEffect_ForceArpeggioSpeed
                   
                                IFDEF PLY_CFG_UseEffect_ForcePitchTableSpeed        ;CONFIG SPECIFIC
        jr PLY_AKM_EffectForcePitchSpeed                         ;111
                                ELSE
                                ;jr $   ;Last one. No need to encode it.
                                ENDIF ;PLY_CFG_UseEffect_ForcePitchTableSpeed             


                                IFDEF PLY_AKM_USE_EffectPitchUpDown        ;CONFIG SPECIFIC
;Pitch up/down effect, activation or stop.
PLY_AKM_EffectPitchUpDown:
        rra     ;Pitch present or pitch stop?
        jr nc,PLY_AKM_EffectPitchUpDown_Deactivated
        ;Activates the effect.
        ld (ix + PLY_AKM_Data_OffsetIsPitchUpDownUsed),255
        ld a,(hl)
        inc hl
        ld (ix + PLY_AKM_Data_OffsetTrackPitchSpeed + 0),a
        ld a,(hl)
        inc hl
        ld (ix + PLY_AKM_Data_OffsetTrackPitchSpeed + 1),a
        jp PLY_AKM_RT_ReadEffect_Return
PLY_AKM_EffectPitchUpDown_Deactivated:
        ;Pitch stop.
        ld (ix + PLY_AKM_Data_OffsetIsPitchUpDownUsed),0
        jp PLY_AKM_RT_ReadEffect_Return
                                ENDIF ;PLY_AKM_USE_EffectPitchUpDown
                        

                                IFDEF PLY_CFG_UseEffect_ArpeggioTable        ;CONFIG SPECIFIC
;Arpeggio table effect, activation or stop.
PLY_AKM_EffectArpeggioTable:
        call PLY_AKM_EffectReadIfEscape         ;Makes sure the data is 0-14, else 15 means: read the next escape value.
        ld (ix + PLY_AKM_Data_OffsetIsArpeggioTableUsed),a       ;Sets to 0 if the Arpeggio is stopped, or any other value if it starts.
        or a
        jr z,PLY_AKM_EffectArpeggioTable_Stop

        ;Gets the Arpeggio address.
        add a,a
        exx
                ld l,a
                ld h,0
        ;BC is modified, will be restored below.
        IFNDEF PLY_AKM_Rom
dknr3 (void):
PLY_AKM_PtArpeggios: ld bc,0            ;Arpeggio table does not encode entry 0, but the pointer points two bytes earlier to compensate.
        ELSE
        ld bc,(PLY_AKM_PtArpeggios)
        ENDIF
                add hl,bc
                ld a,(hl)
                inc hl
                ld h,(hl)
                ld l,a
                ld a,(hl)               ;Reads the speed.
                inc hl
                ld (ix + PLY_AKM_Data_OffsetArpeggioOriginalSpeed),a
                                IFDEF PLY_CFG_UseEffect_ForceArpeggioSpeed        ;CONFIG SPECIFIC
                ld (ix + PLY_AKM_Data_OffsetArpeggioCurrentSpeed),a
                                ENDIF ;PLY_CFG_UseEffect_ForceArpeggioSpeed
                ld (ix + PLY_AKM_Data_OffsetPtArpeggioTable + 0),l
                ld (ix + PLY_AKM_Data_OffsetPtArpeggioTable + 1),h
                
                ld bc,(PLY_AKM_NoteIndexTable + PLY_AKM_Offset1b)
        exx

        ;Resets the offset of the Arpeggio to restart the Arpeggio, and forces a step to read immediately.
        xor a
        ld (ix + PLY_AKM_Data_OffsetPtArpeggioOffset),a
        ld (ix + PLY_AKM_Data_OffsetArpeggioCurrentStep),a
        jp PLY_AKM_RT_ReadEffect_Return
PLY_AKM_EffectArpeggioTable_Stop:
        ;Contrary to the Pitch, the Arpeggio must also be set to 0 when stopped.
        ld (ix + PLY_AKM_Data_OffsetCurrentArpeggioValue),a
        jp PLY_AKM_RT_ReadEffect_Return
                                ENDIF ;PLY_CFG_UseEffect_ArpeggioTable

                                IFDEF PLY_CFG_UseEffect_PitchTable        ;CONFIG SPECIFIC
;Pitch table effect, activation or stop.
;This is almost exactly the same code as for the Arpeggio, but I can't find a way to share it...
PLY_AKM_EffectPitchTable:
        call PLY_AKM_EffectReadIfEscape         ;Makes sure the data is 0-14, else 15 means: read the next escape value.
        ld (ix + PLY_AKM_Data_OffsetIsPitchTableUsed),a  ;Sets to 0 if the Pitch is stopped, or any other value if it starts.
        or a
        jp z,PLY_AKM_RT_ReadEffect_Return
        
        ;Gets the Pitch address.
        add a,a
        exx
                ld l,a
                ld h,0
        ;BC is modified, will be restored below.
        IFNDEF PLY_AKM_Rom
dknr3 (void):
PLY_AKM_PtPitches: ld bc,0            ;Pitch table does not encode entry 0, but the pointer points two bytes earlier to compensate.
        ELSE
        ld bc,(PLY_AKM_PtPitches)
        ENDIF
                add hl,bc
                ld a,(hl)
                inc hl
                ld h,(hl)
                ld l,a
                ld a,(hl)               ;Reads the speed.
                inc hl
                ld (ix + PLY_AKM_Data_OffsetPitchOriginalSpeed),a
                                IFDEF PLY_CFG_UseEffect_ForcePitchTableSpeed        ;CONFIG SPECIFIC
                ld (ix + PLY_AKM_Data_OffsetPitchCurrentSpeed),a
                                ENDIF ;PLY_CFG_UseEffect_ForcePitchTableSpeed
                ld (ix + PLY_AKM_Data_OffsetPtPitchTable + 0),l
                ld (ix + PLY_AKM_Data_OffsetPtPitchTable + 1),h
                
                ld bc,(PLY_AKM_NoteIndexTable + PLY_AKM_Offset1b)
        exx

        ;Resets the offset of the Pitch to restart the Pitch, and forces a step to read immediately.
        xor a
        ld (ix + PLY_AKM_Data_OffsetPtPitchOffset),a
        ld (ix + PLY_AKM_Data_OffsetPitchCurrentStep),a
        jp PLY_AKM_RT_ReadEffect_Return
                                ENDIF ;PLY_CFG_UseEffect_PitchTable





        
;Reads the next escape byte if A is 15, else returns A (0-14).
;IN:    HL= data in the effect
;       A = 0-15. bit 7-4 must be 0.
;OUT:   HL= may be increased if an escape value is read.
;       A = the 8-bit value.
PLY_AKM_EffectReadIfEscape:
        cp 15
        ret c
        ;Reads the escape value.
        ld a,(hl)
        inc hl
        ret

                        ENDIF ;PLY_CFG_UseEffects
                        
                        
;---------------------------------------------------------------------
;Data blocks for the three channels. Make sure NOTHING is added between, as the init clears everything!
;---------------------------------------------------------------------

        counter = 0
        ;Macro to declare a DB if RAM player, or an increasing EQU for ROM player.
        MACRO PLY_AKM_db label
                IFNDEF PLY_AKM_Rom
                        dkbs (void)
                        {label} db 0
                        dkbe (void)
                ELSE
                        {label} equ PLY_AKM_ROM_Buffer + counter
                        counter = counter + 1
                ENDIF
        ENDM
        
        ;Macro to declare a DW if RAM player, or an increasing (of two bytes) EQU for ROM player.
        MACRO PLY_AKM_dw label
                IFNDEF PLY_AKM_Rom
                        dkws (void)
                        {label} dw 0
                        dkwe (void)
                ELSE
                        {label} equ PLY_AKM_ROM_Buffer + counter
                        counter = counter + 2
                ENDIF
        ENDM
        

        ;Specific generic data for ROM (non-related to channels).
        ;Important: must be declared BEFORE the channel-specific data.
        IFDEF PLY_AKM_Rom
        PLY_AKM_dw PLY_AKM_PtInstruments
        PLY_AKM_dw PLY_AKM_PtArpeggios
        PLY_AKM_dw PLY_AKM_PtPitches
        PLY_AKM_dw PLY_AKM_PtLinker
        PLY_AKM_dw PLY_AKM_NoteIndexTable
        PLY_AKM_dw PLY_AKM_TrackIndex
        PLY_AKM_dw PLY_AKM_SaveSP
        
        PLY_AKM_db PLY_AKM_DefaultStartNoteInTracks
        PLY_AKM_db PLY_AKM_DefaultStartInstrumentInTracks
        PLY_AKM_db PLY_AKM_DefaultStartWaitInTracks
        PLY_AKM_db PLY_AKM_PrimaryInstrument
        PLY_AKM_db PLY_AKM_SecondaryInstrument
        PLY_AKM_db PLY_AKM_PrimaryWait
        PLY_AKM_db PLY_AKM_SecondaryWait
        PLY_AKM_db PLY_AKM_FlagNoteAndEffectInCell
        
        PLY_AKM_db PLY_AKM_PatternRemainingHeight
        PLY_AKM_db PLY_AKM_LinkerPreviousRemainingHeight
        PLY_AKM_db PLY_AKM_Speed
        PLY_AKM_db PLY_AKM_TickCounter
        PLY_AKM_db PLY_AKM_SetReg13Old
        PLY_AKM_db PLY_AKM_SetReg13
        PLY_AKM_db PLY_AKM_RT_ReadEffectsFlag
        
        ;RET table: db register, db value, dw code to jump to once the value is read.
        ;MUST be consistent with the RAM buffer!
PLY_AKM_Registers_RetTable: equ PLY_AKM_ROM_Buffer + counter
        ;Reg 8.
        PLY_AKM_db PLY_AKM_Track1_Registers
        PLY_AKM_db PLY_AKM_Track1_Volume
        PLY_AKM_dw PLY_AKM_Track1_VolumeRet
        ;Reg 0.
        PLY_AKM_db PLY_AKM_Track1_SoftwarePeriodLSBRegister
        PLY_AKM_db PLY_AKM_Track1_SoftwarePeriodLSB
        PLY_AKM_dw PLY_AKM_Track1_SoftwarePeriodLSBRet
        ;Reg 1.
        PLY_AKM_db PLY_AKM_Track1_SoftwarePeriodMSBRegister
        PLY_AKM_db PLY_AKM_Track1_SoftwarePeriodMSB
        PLY_AKM_dw PLY_AKM_Track1_SoftwarePeriodMSBRet
        
        ;Reg 9.
        PLY_AKM_db PLY_AKM_Track2_Registers
        PLY_AKM_db PLY_AKM_Track2_Volume
        PLY_AKM_dw PLY_AKM_Track2_VolumeRet
        ;Reg 2.
        PLY_AKM_db PLY_AKM_Track2_SoftwarePeriodLSBRegister
        PLY_AKM_db PLY_AKM_Track2_SoftwarePeriodLSB
        PLY_AKM_dw PLY_AKM_Track2_SoftwarePeriodLSBRet
        ;Reg 3.
        PLY_AKM_db PLY_AKM_Track2_SoftwarePeriodMSBRegister
        PLY_AKM_db PLY_AKM_Track2_SoftwarePeriodMSB
        PLY_AKM_dw PLY_AKM_Track2_SoftwarePeriodMSBRet
        
        ;Reg 10.
        PLY_AKM_db PLY_AKM_Track3_Registers
        PLY_AKM_db PLY_AKM_Track3_Volume
        PLY_AKM_dw PLY_AKM_Track3_VolumeRet
        ;Reg 4.
        PLY_AKM_db PLY_AKM_Track3_SoftwarePeriodLSBRegister
        PLY_AKM_db PLY_AKM_Track3_SoftwarePeriodLSB
        PLY_AKM_dw PLY_AKM_Track3_SoftwarePeriodLSBRet
        ;Reg 5.
        PLY_AKM_db PLY_AKM_Track3_SoftwarePeriodMSBRegister
        PLY_AKM_db PLY_AKM_Track3_SoftwarePeriodMSB
        PLY_AKM_dw PLY_AKM_Track3_SoftwarePeriodMSBRet
        
        IFDEF PLY_AKM_USE_NoiseRegister          ;CONFIG SPECIFIC
                ;Reg 6.
                PLY_AKM_db PLY_AKM_NoiseRegisterPlaceholder
                PLY_AKM_db PLY_AKM_NoiseRegister        ;Misnomer: this is the value.
                PLY_AKM_dw PLY_AKM_NoiseRegisterRet
        ENDIF
        
        ;Reg 7.
        PLY_AKM_db PLY_AKM_MixerRegisterPlaceholder
        PLY_AKM_db PLY_AKM_MixerRegister        ;Misnomer: this is the value.
        IFDEF PLY_CFG_UseHardwareSounds         ;CONFIG SPECIFIC
                PLY_AKM_dw PLY_AKM_MixerRegisterRet
                ;Reg 11.
                PLY_AKM_db PLY_AKM_Reg11Register
                PLY_AKM_db PLY_AKM_Reg11
                PLY_AKM_dw PLY_AKM_Reg11Ret
                ;Reg 12.
                PLY_AKM_db PLY_AKM_Reg12Register
                PLY_AKM_db PLY_AKM_Reg12
                PLY_AKM_dw PLY_AKM_Reg12Ret
                ;This one is a trick to send the register after R13 is managed.
                PLY_AKM_dw PLY_AKM_Reg12Ret2
        ENDIF
        
        PLY_AKM_dw PLY_AKM_RegsFinalRet
        
        
        ;The buffers for sound effects (if any), for each channel. They are treated apart, because they must be consecutive.
                IFDEF PLY_AKM_MANAGE_SOUND_EFFECTS
PLY_AKM_dw PLY_AKM_PtSoundEffectTable
                        REPEAT 3, channelNumber
PLY_AKM_dw PLY_AKM_Channel{channelNumber}_SoundEffectData
PLY_AKM_db PLY_AKM_Channel{channelNumber}_SoundEffectInvertedVolume
PLY_AKM_db PLY_AKM_Channel{channelNumber}_SoundEffectCurrentStep
PLY_AKM_db PLY_AKM_Channel{channelNumber}_SoundEffectSpeed
                if channelNumber != 3
                        counter = counter + 3 ;Padding of 3, but only necessary for channel 1 and 2.
                endif
                        REND
                ENDIF ;PLY_AKM_MANAGE_SOUND_EFFECTS
        
        
        ENDIF ;PLY_AKM_Rom
        
;Data block for channel 1.
        IFNDEF PLY_AKM_Rom
PLY_AKM_Track1_Data:
        ELSE
                counterStartInTrackData = counter                    ;Duplicates the counter value to determine later the size of the track buffer.
        ENDIF
        PLY_AKM_db PLY_AKM_Track1_WaitEmptyCell                      ;How many empty cells have to be waited. 0 = none.
        IFDEF PLY_AKM_Rom
                PLY_AKM_Track1_Data equ PLY_AKM_Track1_WaitEmptyCell
        ENDIF
                        IFDEF PLY_CFG_UseTranspositions         ;CONFIG SPECIFIC
        PLY_AKM_db PLY_AKM_Track1_Transposition
                        ENDIF ;PLY_CFG_UseTranspositions
        PLY_AKM_dw PLY_AKM_Track1_PtStartTrack                       ;Points at the start of the Track to read. Does not change, unless the Track changes.
        PLY_AKM_dw PLY_AKM_Track1_PtTrack                            ;Points on the next Cell of the Track to read. Evolves.
        PLY_AKM_db PLY_AKM_Track1_BaseNote                           ;Base note, such as the note played. The transposition IS included.
        PLY_AKM_db PLY_AKM_Track1_EscapeNote                         ;The escape note. The transposition is NOT included.
        PLY_AKM_db PLY_AKM_Track1_EscapeInstrument                   ;The escape instrument.
        PLY_AKM_db PLY_AKM_Track1_EscapeWait                         ;The escape wait.
        PLY_AKM_dw PLY_AKM_Track1_PtInstrument                       ;Points on the Instrument, evolves.
        PLY_AKM_db PLY_AKM_Track1_InstrumentCurrentStep              ;The current step on the Instrument (>=0, till it reaches the Speed).
        PLY_AKM_db PLY_AKM_Track1_InstrumentSpeed                    ;The Instrument speed (>=0).
                        IFDEF PLY_CFG_UseEffect_SetVolume        ;CONFIG SPECIFIC
        PLY_AKM_db PLY_AKM_Track1_TrackInvertedVolume
                        ENDIF ;PLY_CFG_UseEffect_SetVolume
                        IFDEF PLY_AKM_USE_EffectPitchUpDown        ;CONFIG SPECIFIC
        PLY_AKM_db PLY_AKM_Track1_IsPitchUpDownUsed                  ;>0 if a Pitch Up/Down is currently in use.
        PLY_AKM_dw PLY_AKM_Track1_TrackPitchInteger                   ;The integer part of the Track pitch. Evolves as the pitch goes up/down.
        PLY_AKM_db PLY_AKM_Track1_TrackPitchDecimal                   ;The decimal part of the Track pitch. Evolves as the pitch goes up/down.
        PLY_AKM_dw PLY_AKM_Track1_TrackPitchSpeed                     ;The integer and decimal part of the Track pitch speed. Is added to the Track Pitch every frame.
                        ENDIF ;PLY_AKM_USE_EffectPitchUpDown
                        IFDEF PLY_CFG_UseEffect_ArpeggioTable        ;CONFIG SPECIFIC
        PLY_AKM_db PLY_AKM_Track1_IsArpeggioTableUsed                 ;>0 if an Arpeggio Table is currently in use.
        PLY_AKM_dw PLY_AKM_Track1_PtArpeggioTable                     ;Point on the base of the Arpeggio table, does not evolve.
        PLY_AKM_db PLY_AKM_Track1_PtArpeggioOffset                    ;Increases over the Arpeggio.
        PLY_AKM_db PLY_AKM_Track1_ArpeggioCurrentStep                 ;The arpeggio current step (>=0, increases).
                                IFDEF PLY_CFG_UseEffect_ForceArpeggioSpeed        ;CONFIG SPECIFIC
        PLY_AKM_db PLY_AKM_Track1_ArpeggioCurrentSpeed                ;The arpeggio speed (>=0, may be changed by the Force Arpeggio Speed effect).
                                ENDIF ;PLY_CFG_UseEffect_ForceArpeggioSpeed
        PLY_AKM_db PLY_AKM_Track1_ArpeggioOriginalSpeed               ;The arpeggio original speed (>=0, NEVER changes for this arpeggio).
        PLY_AKM_db PLY_AKM_Track1_CurrentArpeggioValue                ;Value from the Arpeggio to add to the base note. Read even if the Arpeggio effect is deactivated.
                        ENDIF ;PLY_CFG_UseEffect_ArpeggioTable
                        IFDEF PLY_CFG_UseEffect_PitchTable        ;CONFIG SPECIFIC
        PLY_AKM_db PLY_AKM_Track1_IsPitchTableUsed                    ;>0 if a Pitch Table is currently in use.
        PLY_AKM_dw PLY_AKM_Track1_PtPitchTable                        ;Points on the base of the Pitch table, does not evolve.
        PLY_AKM_db PLY_AKM_Track1_PtPitchOffset                       ;Increases over the Pitch.
        PLY_AKM_db PLY_AKM_Track1_PitchCurrentStep                    ;The Pitch current step (>=0, increases).
                                IFDEF PLY_CFG_UseEffect_ForcePitchTableSpeed        ;CONFIG SPECIFIC
        PLY_AKM_db PLY_AKM_Track1_PitchCurrentSpeed                   ;The Pitch speed (>=0, may be changed by the Force Pitch Speed effect).
                                ENDIF ;PLY_CFG_UseEffect_ForcePitchTableSpeed
        PLY_AKM_db PLY_AKM_Track1_PitchOriginalSpeed                  ;The Pitch original speed (>=0, NEVER changes for this pitch).
        PLY_AKM_dw PLY_AKM_Track1_CurrentPitchTableValue              ;16 bit value from the Pitch to add to the base note. Not read if the Pitch effect is deactivated.

                        ENDIF ;PLY_CFG_UseEffect_PitchTable

        IFNDEF PLY_AKM_Rom
PLY_AKM_Track1_Data_End:
PLY_AKM_Track1_Data_Size: equ PLY_AKM_Track1_Data_End - PLY_AKM_Track1_Data
        ELSE
PLY_AKM_Track1_Data_Size = counter - counterStartInTrackData
PLY_AKM_Track1_Data_End = PLY_AKM_Track1_Data + PLY_AKM_Track1_Data_Size
        ENDIF

PLY_AKM_Data_OffsetWaitEmptyCell:                equ PLY_AKM_Track1_WaitEmptyCell - PLY_AKM_Track1_Data
                        IFDEF PLY_CFG_UseTranspositions         ;CONFIG SPECIFIC
PLY_AKM_Data_OffsetTransposition:                equ PLY_AKM_Track1_Transposition - PLY_AKM_Track1_Data
                        ENDIF ;PLY_CFG_UseTranspositions
PLY_AKM_Data_OffsetPtStartTrack:                 equ PLY_AKM_Track1_PtStartTrack - PLY_AKM_Track1_Data
PLY_AKM_Data_OffsetPtTrack:                      equ PLY_AKM_Track1_PtTrack - PLY_AKM_Track1_Data
PLY_AKM_Data_OffsetBaseNote:                     equ PLY_AKM_Track1_BaseNote - PLY_AKM_Track1_Data
PLY_AKM_Data_OffsetEscapeNote:                   equ PLY_AKM_Track1_EscapeNote - PLY_AKM_Track1_Data
PLY_AKM_Data_OffsetEscapeInstrument:             equ PLY_AKM_Track1_EscapeInstrument - PLY_AKM_Track1_Data
PLY_AKM_Data_OffsetEscapeWait:                   equ PLY_AKM_Track1_EscapeWait - PLY_AKM_Track1_Data
PLY_AKM_Data_OffsetSecondaryInstrument:          equ PLY_AKM_Track1_EscapeWait - PLY_AKM_Track1_Data
PLY_AKM_Data_OffsetPtInstrument:                 equ PLY_AKM_Track1_PtInstrument - PLY_AKM_Track1_Data
PLY_AKM_Data_OffsetInstrumentCurrentStep:        equ PLY_AKM_Track1_InstrumentCurrentStep - PLY_AKM_Track1_Data
PLY_AKM_Data_OffsetInstrumentSpeed:              equ PLY_AKM_Track1_InstrumentSpeed - PLY_AKM_Track1_Data
                        IFDEF PLY_CFG_UseEffect_SetVolume        ;CONFIG SPECIFIC
PLY_AKM_Data_OffsetTrackInvertedVolume:          equ PLY_AKM_Track1_TrackInvertedVolume - PLY_AKM_Track1_Data
                        ENDIF ;PLY_CFG_UseEffect_SetVolume
                        IFDEF PLY_AKM_USE_EffectPitchUpDown        ;CONFIG SPECIFIC
PLY_AKM_Data_OffsetIsPitchUpDownUsed:            equ PLY_AKM_Track1_IsPitchUpDownUsed - PLY_AKM_Track1_Data
PLY_AKM_Data_OffsetTrackPitchInteger:            equ PLY_AKM_Track1_TrackPitchInteger - PLY_AKM_Track1_Data
PLY_AKM_Data_OffsetTrackPitchDecimal:            equ PLY_AKM_Track1_TrackPitchDecimal - PLY_AKM_Track1_Data
PLY_AKM_Data_OffsetTrackPitchSpeed:              equ PLY_AKM_Track1_TrackPitchSpeed - PLY_AKM_Track1_Data
                        ENDIF ;PLY_AKM_USE_EffectPitchUpDown
                        IFDEF PLY_CFG_UseEffect_ArpeggioTable        ;CONFIG SPECIFIC
PLY_AKM_Data_OffsetIsArpeggioTableUsed:          equ PLY_AKM_Track1_IsArpeggioTableUsed - PLY_AKM_Track1_Data
PLY_AKM_Data_OffsetPtArpeggioTable:              equ PLY_AKM_Track1_PtArpeggioTable - PLY_AKM_Track1_Data
PLY_AKM_Data_OffsetPtArpeggioOffset:             equ PLY_AKM_Track1_PtArpeggioOffset - PLY_AKM_Track1_Data
PLY_AKM_Data_OffsetArpeggioCurrentStep:          equ PLY_AKM_Track1_ArpeggioCurrentStep - PLY_AKM_Track1_Data
                                IFDEF PLY_CFG_UseEffect_ForceArpeggioSpeed        ;CONFIG SPECIFIC
PLY_AKM_Data_OffsetArpeggioCurrentSpeed:         equ PLY_AKM_Track1_ArpeggioCurrentSpeed - PLY_AKM_Track1_Data
                                ENDIF ;PLY_CFG_UseEffect_ForceArpeggioSpeed
PLY_AKM_Data_OffsetArpeggioOriginalSpeed:        equ PLY_AKM_Track1_ArpeggioOriginalSpeed - PLY_AKM_Track1_Data
PLY_AKM_Data_OffsetCurrentArpeggioValue:         equ PLY_AKM_Track1_CurrentArpeggioValue - PLY_AKM_Track1_Data
                        ENDIF ;PLY_CFG_UseEffect_ArpeggioTable
                        IFDEF PLY_CFG_UseEffect_PitchTable        ;CONFIG SPECIFIC
PLY_AKM_Data_OffsetIsPitchTableUsed:             equ PLY_AKM_Track1_IsPitchTableUsed - PLY_AKM_Track1_Data
PLY_AKM_Data_OffsetPtPitchTable:                 equ PLY_AKM_Track1_PtPitchTable - PLY_AKM_Track1_Data
PLY_AKM_Data_OffsetPtPitchOffset:                equ PLY_AKM_Track1_PtPitchOffset - PLY_AKM_Track1_Data
PLY_AKM_Data_OffsetPitchCurrentStep:             equ PLY_AKM_Track1_PitchCurrentStep - PLY_AKM_Track1_Data
                                IFDEF PLY_CFG_UseEffect_ForcePitchTableSpeed        ;CONFIG SPECIFIC
PLY_AKM_Data_OffsetPitchCurrentSpeed:            equ PLY_AKM_Track1_PitchCurrentSpeed - PLY_AKM_Track1_Data
                                ENDIF ;PLY_CFG_UseEffect_ForcePitchTableSpeed
PLY_AKM_Data_OffsetPitchOriginalSpeed:           equ PLY_AKM_Track1_PitchOriginalSpeed - PLY_AKM_Track1_Data
PLY_AKM_Data_OffsetCurrentPitchTableValue:       equ PLY_AKM_Track1_CurrentPitchTableValue - PLY_AKM_Track1_Data
                        ENDIF ;PLY_CFG_UseEffect_PitchTable

;Data block for channel 2.
        IFNDEF PLY_AKM_Rom
PLY_AKM_Track2_Data:
dkbs (void):
        ds PLY_AKM_Track1_Data_Size, 0
dkbe (void):
PLY_AKM_Track2_Data_End:
        ELSE
PLY_AKM_Track2_Data: equ PLY_AKM_Track1_Data + PLY_AKM_Track1_Data_Size
PLY_AKM_Track2_Data_End: equ PLY_AKM_Track2_Data + PLY_AKM_Track1_Data_Size
        ENDIF
PLY_AKM_Track2_WaitEmptyCell: equ PLY_AKM_Track2_Data + PLY_AKM_Data_OffsetWaitEmptyCell
PLY_AKM_Track2_PtTrack: equ PLY_AKM_Track2_Data + PLY_AKM_Data_OffsetPtTrack
PLY_AKM_Track2_PtInstrument: equ PLY_AKM_Track2_Data + PLY_AKM_Data_OffsetPtInstrument
PLY_AKM_Track2_EscapeNote: equ PLY_AKM_Track2_Data + PLY_AKM_Data_OffsetEscapeNote
PLY_AKM_Track2_EscapeInstrument: equ PLY_AKM_Track2_Data + PLY_AKM_Data_OffsetEscapeInstrument
PLY_AKM_Track2_EscapeWait: equ PLY_AKM_Track2_Data + PLY_AKM_Data_OffsetEscapeWait

;Data block for channel 3.
        IFNDEF PLY_AKM_Rom
PLY_AKM_Track3_Data:
dkbs (void):
        ds PLY_AKM_Track1_Data_Size, 0
dkbe (void):
PLY_AKM_Track3_Data_End:
        ELSE
PLY_AKM_Track3_Data: equ PLY_AKM_Track2_Data + PLY_AKM_Track1_Data_Size
PLY_AKM_Track3_Data_End: equ PLY_AKM_Track3_Data + PLY_AKM_Track1_Data_Size

PLY_AKM_ROM_Buffer_End: equ PLY_AKM_Track3_Data_End
PLY_AKM_ROM_BufferSize: equ PLY_AKM_ROM_Buffer_End - PLY_AKM_ROM_Buffer
        
        expectedRomBufferSize = 199             ;Just to detect if the buffer grows.
                IFNDEF PLY_AKM_MANAGE_SOUND_EFFECTS
                        assert PLY_AKM_ROM_BufferSize <= expectedRomBufferSize               ;Decreases when using the Player Configuration.
                ELSE
                        assert PLY_AKM_ROM_BufferSize <= (expectedRomBufferSize + 23)        ;With sound effects, it takes a bit more memory.
                ENDIF
        ENDIF
        
        
        

PLY_AKM_Track3_WaitEmptyCell: equ PLY_AKM_Track3_Data + PLY_AKM_Data_OffsetWaitEmptyCell
PLY_AKM_Track3_PtTrack: equ PLY_AKM_Track3_Data + PLY_AKM_Data_OffsetPtTrack
PLY_AKM_Track3_PtInstrument: equ PLY_AKM_Track3_Data + PLY_AKM_Data_OffsetPtInstrument
PLY_AKM_Track3_EscapeNote: equ PLY_AKM_Track3_Data + PLY_AKM_Data_OffsetEscapeNote
PLY_AKM_Track3_EscapeInstrument: equ PLY_AKM_Track3_Data + PLY_AKM_Data_OffsetEscapeInstrument
PLY_AKM_Track3_EscapeWait: equ PLY_AKM_Track3_Data + PLY_AKM_Data_OffsetEscapeWait

        ;Makes sure the structure all have the same size!
        ASSERT (PLY_AKM_Track1_Data_End - PLY_AKM_Track1_Data) == (PLY_AKM_Track2_Data_End - PLY_AKM_Track2_Data)
        ASSERT (PLY_AKM_Track1_Data_End - PLY_AKM_Track1_Data) == (PLY_AKM_Track3_Data_End - PLY_AKM_Track3_Data)
        ;No holes between the blocks, the init makes a LDIR to clear everything!
        ASSERT PLY_AKM_Track1_Data_End == PLY_AKM_Track2_Data
        ASSERT PLY_AKM_Track2_Data_End == PLY_AKM_Track3_Data



;---------------------------------------------------------------------
;Register block for all the channels. They are "polluted" with pointers to code because all this
;is actually a RET table!
;---------------------------------------------------------------------
;DB register, DB value then DW code to jump to once the value is read.
        IFNDEF PLY_AKM_Rom              ;For ROM, a table is generated.
PLY_AKM_Registers_RetTable:
PLY_AKM_Track1_Registers:
dkbs (void):   db 8
PLY_AKM_Track1_Volume: db 0
dkbe (void):
dkps (void):   dw PLY_AKM_SendPsgRegister
dkpe (void):

dkbs (void):   db 0
PLY_AKM_Track1_SoftwarePeriodLSB: db 0
dkbe (void):
dkps (void):   dw PLY_AKM_SendPsgRegister
dkpe (void):

dkbs (void):   db 1
PLY_AKM_Track1_SoftwarePeriodMSB: db 0
dkbe (void):
dkps (void):   dw PLY_AKM_SendPsgRegister
dkpe (void):

PLY_AKM_Track2_Registers:
dkbs (void):   db 9
PLY_AKM_Track2_Volume: db 0
dkbe (void):
dkps (void):   dw PLY_AKM_SendPsgRegister
dkpe (void):

dkbs (void):   db 2
PLY_AKM_Track2_SoftwarePeriodLSB: db 0
dkbe (void):
dkps (void):   dw PLY_AKM_SendPsgRegister
dkpe (void):

dkbs (void):   db 3
PLY_AKM_Track2_SoftwarePeriodMSB: db 0
dkbe (void):
dkps (void):   dw PLY_AKM_SendPsgRegister
dkpe (void):


PLY_AKM_Track3_Registers:
dkbs (void):   db 10
PLY_AKM_Track3_Volume: db 0
dkbe (void):
dkps (void):   dw PLY_AKM_SendPsgRegister
dkpe (void):

dkbs (void):   db 4
PLY_AKM_Track3_SoftwarePeriodLSB: db 0
dkbe (void):
dkps (void):   dw PLY_AKM_SendPsgRegister
dkpe (void):

dkbs (void):   db 5
PLY_AKM_Track3_SoftwarePeriodMSB: db 0
dkbe (void):
dkps (void):   dw PLY_AKM_SendPsgRegister
dkpe (void):

;Generic registers.
                        IFDEF PLY_AKM_USE_NoiseRegister          ;CONFIG SPECIFIC
dkbs (void):   db 6
PLY_AKM_NoiseRegister: db 0
dkbe (void):
dkps (void):   dw PLY_AKM_SendPsgRegister
dkpe (void):
                        ENDIF ;PLY_AKM_USE_NoiseRegister

dkbs (void):   db 7
PLY_AKM_MixerRegister: db 0
dkbe (void):
        IFDEF PLY_CFG_UseHardwareSounds         ;CONFIG SPECIFIC
dkps (void):           dw PLY_AKM_SendPsgRegister
dkpe (void):
        
dkbs (void):    db 11
PLY_AKM_Reg11:  db 0
dkbe (void):
dkps (void):           dw PLY_AKM_SendPsgRegister
dkpe (void):

dkbs (void):    db 12
PLY_AKM_Reg12:  db 0
dkbe (void):
dkps (void):           dw PLY_AKM_SendPsgRegisterR13
                ;This one is a trick to send the register after R13 is managed.
                dw PLY_AKM_SendPsgRegisterAfterPop
dkpe (void):
        ENDIF ;PLY_CFG_UseHardwareSounds
dkps (void):   dw PLY_AKM_SendPsgRegisterEnd
dkpe (void):

        ENDIF ;PLY_AKM_Rom


PLY_AKM_Registers_OffsetVolume: equ PLY_AKM_Track1_Volume - PLY_AKM_Track1_Registers
PLY_AKM_Registers_OffsetSoftwarePeriodLSB: equ PLY_AKM_Track1_SoftwarePeriodLSB - PLY_AKM_Track1_Registers
PLY_AKM_Registers_OffsetSoftwarePeriodMSB: equ PLY_AKM_Track1_SoftwarePeriodMSB - PLY_AKM_Track1_Registers

;The period table for the first octave only.
PLY_AKM_PeriodTable:
dkws (void):
        IFDEF PLY_AKM_HARDWARE_CPC
        ;PSG running to 1000000 Hz.
         dw 3822,3608,3405,3214,3034,2863,2703,2551,2408,2273,2145,2025          ; Octave 0.
        ;dw 1911,1804,1703,1607,1517,1432,1351,1276,1204,1136,1073,1012          ;12
        ;dw  956, 902, 851, 804, 758, 716, 676, 638, 602, 568, 536, 506          ;24
        ;dw  478, 451, 426, 402, 379, 358, 338, 319, 301, 284, 268, 253          ;36
        ;dw  239, 225, 213, 201, 190, 179, 169, 159, 150, 142, 134, 127          ;48
        ;dw  119, 113, 106, 100,  95,  89,  84,  80,  75,  71,  67,  63          ;60
        ;dw   60,  56,  53,  50,  47,  45,  42,  40,  38,  36,  34,  32          ;72
        ;dw   30,  28,  27,  25,  24,  22,  21,  20,  19,  18,  17,  16          ;84
        ;dw   15,  14,  13,  13,  12,  11,  11,  10,   9,   9,   8,   8          ;96
        ;dw    7,   7,   7,   6,   6,   6,   5,   5,   5,   4,   4,   4          ;108
        ;dw    4,   4,   3,   3,   3,   3,   3,   2  ;,2,   2,   2,   2          ;120 -> 127
        ENDIF

        IFDEF PLY_AKM_HARDWARE_SPECTRUM_OR_MSX
        ;PSG running to 1773400 Hz.
        dw 6778, 6398, 6039, 5700, 5380, 5078, 4793, 4524, 4270, 4030, 3804, 3591	; Octave 0.
        ENDIF

        IFDEF PLY_AKM_HARDWARE_PENTAGON
        ;PSG running to 1750000 Hz.
        dw 6689, 6314, 5959, 5625, 5309, 5011, 4730, 4464, 4214, 3977, 3754, 3543	; Octave 0.
        ENDIF
dkwe (void):

player_end_song:
    AKMSTS = #C011
    AKMLOOP = #C012
    ld a, (AKMLOOP)
    or #80             ; set bit 7
    ld (AKMLOOP), a
    and 1              ; test bit 0 (1=loop)
    ret nz
      ld a, 3          ; stop song
      ld (AKMSTS), a
      ret

      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop

PLY_AKM_End:



