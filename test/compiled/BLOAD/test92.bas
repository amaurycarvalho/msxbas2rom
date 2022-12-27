10 SCREEN 7,3

20 BLOAD "FANGIRL3.SC7", S
21 COLOR=RESTORE
22 BLOAD "sprite2.spr", S

30 X% = 100 : Y% = 80 : S% = 2

40 PUT SPRITE 0, (X%, Y%),,S%
50 PUT SPRITE 1, (X%, Y%),,S%+1

70 K% = STICK(0) OR STICK(1)
80 IF K% = 7 THEN S% = 0 : X% = X% - 1 : GOTO 40
90 IF K% = 3 THEN S% = 4 : X% = X% + 1 : GOTO 40
100 IF K% = 1 THEN S% = 2 : Y% = Y% - 1 : GOTO 40
110 IF K% = 5 THEN S% = 2 : Y% = Y% + 1 : GOTO 40

120 GOTO 70

