10 OPEN "GRP:" FOR OUTPUT AS #1
20 SCREEN 5 : COLOR15,1,1 : CLS
25 CALL TURBO ON
30 LINE(35,60)-(210,125),4,BF:COLOR 10,0
40 PRESET(65,70),,TPSET
45 CALL TURBO OFF
50 PRINT#1,"TESTE"
60 A$ = INPUT$(1)
70 SCREEN 0

