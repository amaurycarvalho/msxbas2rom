' https://www.msx.org/wiki/WIDTH

10 SCREEN 0 
11 PRINT "SCREEN 0 - DEFAULT" 
12 GOSUB 100

20 WIDTH 20 
21 PRINT "SCREEN 0 - WIDTH 20" 
22 GOSUB 100

30 SCREEN 1 
31 PRINT "SCREEN 1 - DEFAULT"
32 GOSUB 100

40 WIDTH 20 
41 PRINT "SCREEN 1 - WIDTH 20"
42 GOSUB 100

50 END

100 PRINT
101 PRINT "0        1         2         3         4"
102 PRINT "1234567890123456789012345678901234567890"
103 A$ = INPUT$(1)
104 RETURN
