10 SCREEN 1
20 PRINT BASE(5), BASE(6), BASE(7), BASE(8)
30 PRINT BASE(10), BASE(11), BASE(12), BASE(13)
40 POKE &HCF00, 25
50 PRINT PEEK(1000), PEEK(&HCF00)
60 VPOKE BASE(5), 33 
70 PRINT VPEEK(BASE(5))
80 VDP(17)=7   ' modify color 7
90 OUT &H9A,69: OUT &H9A,3   
100 T% = TIME
110 IF (TIME-T%) < 100 THEN 110
120 PRINT TIME, T%
130 COLOR 7


