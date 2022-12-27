10 CLS 
20 INPUT "Your name"; N$
30 PRINT "Press a key"
40 A$ = INKEY$ : IF A$ = "" THEN GOTO 40
50 B$ = STR$(10)
60 PRINT N$, A$; " "; STRING$(5, "=");"> ";B$
70 A=INP(&HA8)
80 A$="00000000"+BIN$(A)
90 PRINT RIGHT$(A$,8)

