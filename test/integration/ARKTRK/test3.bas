FILE "songs.akm"
FILE "effects.akx"

10 CMD PLYLOAD 0, 1
11 CMD PLYSONG 0
12 CMD PLYLOOP 0         ' disable loop in songs
13 CMD PLYPLAY

20 PRINT "Playing song..."

30 S% = PLYSTATUS()
31 IF S% < 128 THEN 30

40 CMD PLYMUTE
41 PRINT "Song finished"

50 INPUT "Replay (Y/N)"; R$
51 IF R$ <> "Y" AND R$ <> "y" THEN END
52   PRINT "Playing again..."
53   CMD PLYREPLAY
54   GOTO 30 


