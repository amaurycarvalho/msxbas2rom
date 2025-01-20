10 CLS
20 PRINT "<<< Random generator >>>"
30 LINE INPUT "Press ENTER to RANDOMIZE..."; A 
40 RANDOMIZE
45 TIME = 1000
50 LINE INPUT "Press ENTER to next RND..."; A
60 A = TIME
70 B# = RND(1)
80 PRINT A; " => "; B#
90 A = TIME
100 B# = RND(1)
110 PRINT A; " => "; B#
120 A = TIME
130 B# = RND(1)
140 PRINT A; " => "; B#
150 A = TIME
160 B# = RND(1)
170 PRINT A; " => "; B#
180 GOTO 50

