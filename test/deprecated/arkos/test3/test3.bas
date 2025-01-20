' ARKOS TEST - 48kb AND 128kb ROMs

FILE "test3a1.akm"
FILE "test3.akx"

10 PRINT "Loading songs"
20 CMD PLYLOAD 0, 1
30 PRINT "Playing a song"
40 CMD PLYSONG 0
50 CMD PLYPLAY
60 LINE INPUT "Press ENTER to stop...", A$
70 CMD PLYMUTE
80 END

