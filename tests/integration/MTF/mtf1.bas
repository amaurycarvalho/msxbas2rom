' ----------------------------------------------------------------------------
' MSX TILE FORGE - MAP TO SCREEN FULL COPY (SCREEN 2)
' ----------------------------------------------------------------------------
' MTF <resource>, 0, <horizontal_col>, <vertical_row>
'   horizontal_col  Horizontal screen position in the map (default=0)
'   vertical_row    Vertical screen position in the map (default=0)
' ----------------------------------------------------------------------------

FILE "mtf.SC4Pal"           ' 0
FILE "mtf.SC4Tiles"         ' 1
FILE "mtf.SC4Map"           ' 2 (.SC4Super it's also included automatically)

10 SCREEN 2

20 CMD MTF 0                ' load palettes
21 CMD MTF 1                ' load tiles

30 CMD MTF 2                ' load map's 1st col and 1st row screen (0,0) with relative coords (operation=0)
31 A$ = INPUT$(1)

40 CMD MTF 2, 0, 1          ' load map's 2nd col and 1st row screen (1,0) with relative coords (operation=0)
41 A$ = INPUT$(1)

50 CMD MTF 2, 0, 2          ' load map's 3rd col and 1st row screen (2,0) with relative coords (operation=0)
51 A$ = INPUT$(1)

60 CMD MTF 2, 0, 0, 1       ' load map's 1st col and 2nd row screen (0,1) with relative coords (operation=0)
61 A$ = INPUT$(1)

70 CMD MTF 2, 0, 2, 1       ' load map's 3rd col and 2nd row screen (2,1) with relative coords (operation=0)
71 A$ = INPUT$(1)

80 CMD MTF 2, 0, 1, 2       ' load map's 2nd col and 3rd row screen (1,2) with relative coords (operation=0)
81 A$ = INPUT$(1)

