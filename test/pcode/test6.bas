FILE "test6.spr.plet5"
TEXT "EEDCFFEDEGFEDDD"
TEXT "FFEDEEDCDGFDCCC"
10 SCREEN 2, 3 : KEY OFF
20 X% = 50 : Y% = 50
30 CMD WRTSPRPAT 0
35 SPRITE$(1) = STRING$(8,CHR$(255))
40 PUT SPRITE 0,(X%,Y%),8
50 CMD PLAY 1
51 IF PLAY(0) GOTO 51
52 PUT SPRITE 0,,2
60 CMD PLAY 2
61 IF PLAY(0) GOTO 61
62 PUT SPRITE 0,(,),,1
63 FOR I = 1 TO 1000 : NEXT
64 PUT SPRITE 0,(),6
65 FOR I = 1 TO 1000 : NEXT
70 PUT SPRITE 0,(X%,Y%),15,0
80 K% = STICK(0) + STICK(1)
90 IF K% = 1 THEN Y% = Y% - 2 : GOTO 70
100 IF K% = 5 THEN Y% = Y% + 2 : GOTO 70
110 IF K% = 7 THEN X% = X% - 2 : GOTO 70
120 IF K% = 3 THEN X% = X% + 2 : GOTO 70
130 IF NOT STRIG(0) THEN GOTO 80
140 SCREEN 0 
