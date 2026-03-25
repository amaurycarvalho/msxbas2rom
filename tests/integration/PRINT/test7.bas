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
110 PRINT "Integer to Variant (Single): "; E
120 B% = "123"
130 PRINT "Cast String to Integer: "; B%
140 A$ = B%
150 PRINT "Cast Integer to String: "; A$
160 B% = 3.1415
170 PRINT "Cast Single to Integer: "; B%
180 C! = B%
190 PRINT "Cast Integer to Single: "; C!
200 A$ = C!
210 PRINT "Cast Single to String: "; A$
220 C! = "-1.5"
230 PRINT "Cast String to Single: "; C!
240 D# = "1.500001"
250 PRINT "Cast String to Double: "; C!

