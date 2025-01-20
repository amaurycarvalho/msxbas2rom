10 FOR I = 0 TO 4: STRIG(I) ON: NEXT
20 ON STRIG GOSUB 90, 100, 110, 120, 130
30 CLS
40 PRINT"Press spacebar or a button joystick"
50 PRINT
60 GOTO 80
70 PRINT " pressed"
80 GOTO 80
90 PRINT "Spacebar";:RETURN 70
100 PRINT "Button 1 of joystick 1";:RETURN 70
110 PRINT "Button 1 of joystick 2";:RETURN 70
120 PRINT "Button 2 of joystick 1";:RETURN 70
130 PRINT "Button 2 of joystick 2";:RETURN 70

