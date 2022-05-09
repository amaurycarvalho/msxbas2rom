05 ' FUNCTION KEYS TEST
10 FOR I = 1 TO 3: KEY(I) STOP: NEXT ' F1 to F3 are stopped
20 FOR I = 4 TO 10: KEY(I) OFF: NEXT ' F4 to F10 are disabled
30 ON KEY GOSUB 170, 180, 190 ' Subroutine will only be active from line 130
40 CLS
50 PRINT"TEST 1" 
60 PRINT"Press a function key or space..."
70 FOR I = 1 TO 10 : KEY I,"OFF" : NEXT : KEY ON
80 IF STRIG(0) = 0 THEN 80
90 CLS
100 PRINT"TEST 2"
110 PRINT"Press a function key or space"
120 FOR I = 1 TO 10 : KEY I,"F"+STR$(I) : NEXT 
130 KEY(1) ON : KEY(2) ON : KEY(3) ON ' F1 to F3 and the subroutine in line 30 are enabled
140 IF STRIG(0) = 0 THEN 140
150 KEY OFF : END
170 PRINT "F1 pressed" : RETURN 
180 PRINT "F2 pressed" : RETURN
190 PRINT "F3 pressed" : RETURN
