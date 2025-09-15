' MSX TILE FORGE
' TEST SCREEN 2

FILE "mtf.SC4Pal"           ' 0
FILE "mtf.SC4Tiles"         ' 1
FILE "mtf.SC4Map"           ' 2 (.SC4Super it's also included automatically)

10 SCREEN 2

20 CMD MTF 0                ' load palettes
21 CMD MTF 1                ' load tiles

30 CMD MTF 2                ' load map's 1st screen
31 A$ = INPUT$(1)

40 CMD MTF 2, 1             ' load map's 2nd screen 
41 A$ = INPUT$(1)

50 FOR I% = 2 TO 11
51   CMD MTF 2, I%          ' load map screen number I%
52   A$ = INPUT$(1)
53 NEXT



