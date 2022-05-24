' SCROLL DEMONSTRATION 3
' by: Amaury Carvalho (2022)

FILE "STARS.SC4"     ' screen resource
FILE "SHIP.SPR"      ' tiny sprite resource

10 DIM SC%(15, 23)   ' 32 cols (0-15: 2 cols for each integer), 24 rows (0-23)

20 SCREEN 4, 2

30 SCREEN LOAD 0     ' load 1st resource on screen
40 SPRITE LOAD 1     ' load 2nd resource on sprite bank

50 PUT SPRITE 0, (100, 100),,0
51 PUT SPRITE 1, (100, 100),,1

60 SCREEN COPY TO SC% SCROLL 3            ' copy to array and do a scroll on the data copied

70 FOR Y% = 0 TO 17
71   R% = ((RND(1) * 100) MOD 30) + 32
72   SC%(15, Y%) = R%                     ' change last column two bytes
74 NEXT 

80 SCREEN PASTE FROM SC%                  ' copy from array to screen

90 GOTO 60



