10 CLS
20 Y% = 5
30 X% = 10
40 LOCATE X%, Y%
50 PRINT "~~~~~(0v0)~~~~~";
60 A% = INKEY
70 IF A% = 0 THEN GOTO 60
80 LOCATE X%, Y%
90 PRINT "               ";
100 IF A% = 28 THEN X% = X% + 1
110 IF A% = 29 THEN X% = X% - 1
120 IF A% = 30 THEN Y% = Y% - 1
130 IF A% = 31 THEN Y% = Y% + 1
140 GOTO 40
