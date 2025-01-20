FILE "sprite1.spr"

10 SCREEN 2,3

20 CMD WRTSPR 0

30 X% = 100 : Y% = 80 : S% = 3
31 PRINT "SCREEN 2"
32 CIRCLE(120,100),50,15

40 PUT SPRITE 0, (X%, Y%),,S%
50 PUT SPRITE 1, (X%, Y%),,S%+1
60 PUT SPRITE 2, (X%, Y%),,S%+2

70 K% = STICK(0) OR STICK(1)
80 IF K% = 7 THEN S% = 0 : X% = X% - 1 : GOTO 40
90 IF K% = 3 THEN S% = 6 : X% = X% + 1 : GOTO 40
100 IF K% = 1 THEN S% = 3 : Y% = Y% - 1 : GOTO 40
110 IF K% = 5 THEN S% = 3 : Y% = Y% + 1 : GOTO 40

120 GOTO 70

