' ----------------------------------------------------------------------------
' MSX TILE FORGE - MAP TO SCREEN PARTIAL SCROLL
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
FILE "mtf.SC4Map"           ' 2 (.SC4Super it's also included automatically)

10 SCREEN 2, 2, 0
20 CMD MTF 0                ' load palettes
30 CMD MTF 1                ' load tiles

40 X% = 0 : Y% = 0
41 CLS

' Copy a 20x16 tile window from map position {x,u}
' to screen position {6,4}
50 CMD MTF 2, 1, X%, Y%, 15, 15, 6, 4

60 K% = STICK(0) OR STICK(1)
61 IF INKEY = 27 THEN END
62 IF K% = 0 THEN 60

70 IF K% = 1 OR K% = 2 OR K% = 8 THEN Y% = Y% - 1 
71 IF K% = 4 OR K% = 5 OR K% = 6 THEN Y% = Y% + 1
72 IF K% = 6 OR K% = 7 OR K% = 8 THEN X% = X% - 1
73 IF K% = 2 OR K% = 3 OR K% = 4 THEN X% = X% + 1
74 GOTO 50


