FILE "mtf.SC4Pal"           ' 0
FILE "mtf.SC4Tiles"         ' 1
FILE "mtf.SC4Map"           ' 2

10 SCREEN 2

20 CMD MTF 0                ' load palette from resource 0
21 CMD MTF 1                ' load tileset from resource 1

30 CMD MTF 2                ' load first screen from the map
31 A$ = INPUT$(1)

40 ' Copy a 10x6 tile window from map position {20,12}
41 ' to screen position {5,8}
42 CMD MTF 2, 2, 20, 12, 10, 6, 5, 8
43 A$ = INPUT$(1)

50 ' Copy an 8x4 tile window from map position {40,20}
51 ' to screen position {0,0}
52 CMD MTF 2, 2, 40, 20, 8, 4, 0, 0
53 A$ = INPUT$(1)

60 ' Copy a 16x10 tile window from map position {0,32}
61 ' to screen position {8,4}
62 CMD MTF 2, 2, 0, 32, 16, 10, 8, 4
63 A$ = INPUT$(1)


