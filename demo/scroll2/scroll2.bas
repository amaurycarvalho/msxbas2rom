' SCROLL DEMONSTRATION 2
' by: Amaury Carvalho (2022)

FILE "STARS.SC2"     ' screen resource
FILE "SHIP.SPR"      ' tiny sprite resource

10 SCREEN 2, 2

20 SCREEN LOAD 0     ' load 1st resource on screen
30 SPRITE LOAD 1     ' load 2nd resource on sprite bank

40 PUT SPRITE 0, (100, 100), 15

50 S% = STICK(0)

60 IF S% = 3 OR S% = 7 THEN SCREEN SCROLL S%

70 GOTO 50



