10 DIM G%(10,14), B%(23)
15 DIM W#(10,14), Z#(23)

20 X% = 9 : Y% = 11

30 CLS

100 TI# = TIME
101 FOR I% = 0 TO 10000
102   N% = B%(9)
103 NEXT I%
104 PRINT "1D INTEGER DIRECT="; TIME - TI#;

105 TI# = TIME
106 FOR I% = 0 TO 10000
107   N% = B%(X%)
108 NEXT I%
109 PRINT ", INDIRECT="; TIME - TI#

200 TI# = TIME
201 FOR I% = 0 TO 10000
202   N% = G%(9, 11)
203 NEXT I%
204 PRINT "2D INTEGER DIRECT="; TIME - TI#;

205 TI# = TIME
206 FOR I% = 0 TO 10000
207   N% = G%(X%, Y%)
208 NEXT I%
209 PRINT ", INDIRECT="; TIME - TI#

300 TI# = TIME
301 FOR I% = 0 TO 10000
302   V# = Z#(X%)
303 NEXT I%
304 PRINT "1D FLOAT INDIRECT = "; TIME - TI#

305 TI# = TIME
306 FOR I% = 0 TO 10000
307   V# = W#(X%, Y%)
308 NEXT I%
309 PRINT "2D FLOAT INDIRECT = "; TIME - TI#

500 TI# = TIME
501 FOR I% = 0 TO 10000
502   'N% = 1
503 NEXT I%
504 PRINT "INTEGER FOR/NEXT = "; TIME - TI#

600 TI# = TIME
601 FOR V# = 0 TO 10000
602   'N% = 1
603 NEXT V#
604 PRINT "FLOAT FOR/NEXT = "; TIME - TI#


