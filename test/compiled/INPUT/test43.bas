05 DEF USR = &h00C3
10 DEF USR0 = &h00C3
20 DEF USR1 = &h00C3
30 DEF USR2 = &h00C3
35 LINE INPUT "LINE INPUT test:";A$
36 PRINT A$
37 INPUT "INPUT test:";A$
38 PRINT A$ : A$ = INPUT$(1)
40 X% = USR0(0) : PRINT "USR";tab(15);"0" : A$=INPUT$(1)
50 X% = USR1(0) : PRINT "USR";tab(15);"1" : A$=INPUT$(1)
60 X% = USR2(0) : PRINT "USR";tab(15);"2" : A$=INPUT$(1)
