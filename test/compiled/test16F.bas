10 CLS
20 PRINT "<<<< IF - COMPARE FLOATS >>>>"
30 INPUT "VALUE A"; A#
40 INPUT "VALUE B"; B#
41 C# = -3.14
45 PRINT "A = "; A#; " B = "; B#; " C = "; C#
50 IF A# = 0 AND B# = 0 THEN END
60 IF A# = B# THEN PRINT "A IS EQUAL TO B"
70 IF A# > B# THEN PRINT "A IS GREATER THAN B" ELSE IF A# < B# THEN PRINT "A IS LESS THAN B"
80 GOTO 30

