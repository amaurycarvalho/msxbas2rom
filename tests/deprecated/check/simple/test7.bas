10 CLS
20 A$ = "Lorem ipsum dolor sit amet."
30 B% = 2021
40 C! = 3.1415
50 D# = 3.14159265359
60 PRINT "Fixed String: ";A$
70 PRINT "Fixed Integer: ";B%
80 PRINT "Fixed Single: ";C!
90 PRINT "Fixed Double: ";D#
100 LET E = B%
110 PRINT "VarInt to VarSingle: "; E
120 B% = "123"
130 PRINT "Cast String to Int "; B%
140 A$ = B%
150 PRINT "Cast Int to String: "; A$
160 B% = 3.1415
170 PRINT "Cast Single to Int: "; B%
180 C! = B%
190 PRINT "Cast Int to Single: "; C!
200 A$ = C!
210 PRINT "Cast Single to String: "; A$
220 D! = "-1.5"
230 PRINT "Cast String to Single: "; D!

