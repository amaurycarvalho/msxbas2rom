FILE "songs.akm"    ' resource 0
FILE "effects.akx"  ' resource 1

10 CMD PLYLOAD 0, 1
11 CMD PLYSONG 0
12 CMD PLYPLAY

20 CLS 
21 PRINT "Playing song..."

30 K% = INKEY
31 IF K% = 0 THEN 30
32 IF K% = 27 THEN 50

40 CMD PLYSOUND 1,2
41 IF INKEY <> 0 THEN 41
42 GOTO 30

50 CMD PLYMUTE
51 PRINT "Song stopped"
52 END

