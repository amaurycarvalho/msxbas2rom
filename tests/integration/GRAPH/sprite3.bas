' OR color test
' https://www.msx.org/wiki/The_OR_Color
' https://www.msx.org/wiki/PUT_SPRITE
' Regression probe: on Panasonic_FS-A1WX SCREEN 4, sprite 1's colour table
' is at VRAM 0x1C10..0x1C1F and must contain 16 bytes of 0x6C
' (OR + collision ignore + colour 12). This address is layout-specific;
' adjust the manifest probe and baseline together if the layout changes.

10 COLOR15,15,0:SCREEN 4
20 SPRITE$(0)=STRING$(8,CHR$(&B11111111)):SPRITEON
30 PUTSPRITE0,(100,100),1,0 ' blue
40 PUTSPRITE1,(103,100),12+96,0 ' green
50 A$=INPUT$(1)

