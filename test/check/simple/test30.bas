10 CLS
20 PRINT "ARROW CLICKED:"
30 LOCATE 15,0 : IF STRIG(0) THEN END
40 ON STICK(0) GOSUB 70,,100,,80,,90
50 GOSUB 60:GOTO 30
60 PRINT "     ":RETURN
70 PRINT "UP   ":RETURN
80 PRINT "DOWN ":RETURN
90 PRINT "LEFT ":RETURN
100 PRINT "RIGHT":RETURN
