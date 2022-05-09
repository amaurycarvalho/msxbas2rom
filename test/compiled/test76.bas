10 FOR I=1 TO 3: KEY(I) STOP: NEXT ' F1 to F3 are stopped
20 FOR I=4 TO 10: KEY(I) OFF: NEXT ' F4 to F10 are disabled
30 ON KEY GOSUB 170,180,190 ' Subroutine will only be active from line 130
40 CLS
50 PRINT"TEST 1: No effect"
60 PRINT
70 PRINT"Press a function key or ESC key"
80 A$=INPUT$(1)
90 IF A$=CHR$(27) THEN 100 ELSE 80
100 CLS
110 PRINT"TEST 2"
120 PRINT"Press a function key"
130 KEY(1)ON: KEY(2)ON: KEY(3)ON ' F1 to F3 and the subroutine in line 30 are enabled
140 GOTO 160
150 PRINT:PRINT a$+" pressed"
160 GOTO 160
170 A$="F1": RETURN 150
180 A$="F2": RETURN 150
190 A$="F3": RETURN 150
