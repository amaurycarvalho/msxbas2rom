INCLUDE "test7.inc"
10 SCREEN 2, 3 : KEY OFF
20 X% = 50 : Y% = 50
30 CMD WRTSPRPAT 0
40 PUT SPRITE 0,(X%,Y%),8
50 CMD PLAY 1
60 CMD PLAY 2
70 PUT SPRITE 0,(X%,Y%),15
80 K% = STICK(0) + STICK(1)
90 IF K% = 1 THEN Y% = Y% - 2 : GOTO 70
100 IF K% = 5 THEN Y% = Y% + 2 : GOTO 70
110 IF K% = 7 THEN X% = X% - 2 : GOTO 70
120 IF K% = 3 THEN X% = X% + 2 : GOTO 70
130 IF STRIG(0) = 0 THEN GOTO 80
140 SCREEN 0 