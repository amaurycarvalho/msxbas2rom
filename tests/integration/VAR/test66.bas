10 DIM G%(10,14), B%(23)
11 DIM S$(3,2), V#(10,14)

20 I% = 0
30 I% = I% + 1
40 I% = I% - 1

50 FOR I% = 0 TO 23
60   B%(I%) = I%
70 NEXT

80 I% = 9 : K% = 5
90 G%(I%,K%) = 11

100 N% = G%(I%, K%)
105 M% = B%(I%)

110 K% = B%(9)
120 I% = G%(9,5)
121 J! = 2
122 L# = 3

200 FOR X% = 0 TO 23
201   B%(X%) = X%
202 NEXT

210 FOR X% = 0 TO 23
211   PRINT B%(X%);" ";
212 NEXT
213 A$ = INPUT$(1)

220 I% = 0 : PRINT : PRINT
221 FOR X% = 0 TO 10
222   FOR Y% = 0 TO 14
223     G%(X%, Y%) = I% : V#(X%, Y%) = I% : I% = I% + 1
224   NEXT
225 NEXT

231 FOR X% = 0 TO 10
232   FOR Y% = 0 TO 14
233     PRINT G%(X%, Y%); " "; V#(X%, Y%); " ";
234   NEXT
235 NEXT
236 PRINT

300 S$(0,0) = "ABC" : S$(0,1) = "CDE"
301 S$(3,1) = "AAA" : S$(3,0) = "BBB"
302 X% = 3 : Y% = 2 : S$(X%,Y%) = "EEE"
310 FOR X% = 0 TO 3 
311   FOR Y% = 0 TO 2
312     PRINT S$(X%,Y%); "-";
313   NEXT
314 NEXT
315 PRINT

400 FOR X% = 100 TO 0 STEP -2
401   PRINT X%,
402 NEXT


