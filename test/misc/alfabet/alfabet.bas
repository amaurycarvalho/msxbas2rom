FILE "ALFABETO.ALF.plet5"
10 S% = 1 : GOSUB 100
20 S% = 2 : GOSUB 100
30 S% = 5 : GOSUB 100
40 S% = 8 : GOSUB 100
50 SCREEN 1
60 END

100 SCREEN S%
101 CMD WRTFNT 0
102 LOCATE 10, 10 : PRINT "SCREEN";S% : A$ = INPUT$(1)
103 RETURN
