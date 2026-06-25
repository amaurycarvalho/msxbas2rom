' ----------------------------------------------------------------------------
' MSX TILE FORGE - MAP TO SCREEN PARTIAL COPY
' ----------------------------------------------------------------------------
' CMD MTF MTF <resource>,2,<map_x>,<map_y>,<width>,<height>,<screen_x>,<screen_y>
'   map_x           Source X coordinate in the map
'   map_y           Source Y coordinate in the map
'   width           Window width in tiles
'   height          Window height in tiles
'   screen_x        Destination X coordinate on screen
'   screen_y        Destination Y coordinate on screen
' ----------------------------------------------------------------------------

FILE "mtf.SC4Pal"           ' 0
FILE "mtf.SC4Tiles"         ' 1
FILE "mtf.SC4Map"           ' 2

10 SCREEN 2

20 CMD MTF 0                ' load palette from resource 0
21 CMD MTF 1                ' load tileset from resource 1

30 CMD MTF 2                ' load first screen from the map
31 A$ = INPUT$(1)

' Copy a 16x4 tile window from map position {36,4}
' to screen position {10,14}
40 CMD MTF 2, 2, 36, 4, 16, 4, 10, 14
41 A$ = INPUT$(1)

' Copy an 8x8 tile window from map position {52,52}
' to screen position {10,2}
50 CMD MTF 2, 2, 52, 52, 8, 8, 10, 2
51 A$ = INPUT$(1)

' Copy a 8x16 tile window from map position {68,28}
' to screen position {1,1}
60 CMD MTF 2, 2, 68, 28, 8, 16, 1, 1
61 A$ = INPUT$(1)

' Copy an 8x8 tile window from map position {96,72}
' to screen position {20,4}
70 CMD MTF 2, 2, 96, 72, 8, 8, 20, 4
71 A$ = INPUT$(1)

