10 CLS
20 PRINT "<<<< INKEY FUNCTION TEST >>>>"
30 PRINT "SPACE = CHR / ASC"
40 M = 0
50 IF M = 0 THEN A% = INKEY()
60 IF M = 0 THEN IF A% > 0 THEN IF A% = 32 THEN PRINT "SPACE" : M = 1 ELSE PRINT A%
70 IF M = 1 THEN B$ = INKEY$()
80 IF M = 1 THEN IF B$ <> "" THEN IF B$ = " " THEN PRINT "SPACE" : M = 0 ELSE PRINT B$
90 GOTO 50
