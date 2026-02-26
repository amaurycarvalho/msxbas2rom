10 CLS
20 BEEP
30 PRINT "Playing note C..."
40 ' Plays the note C on channel A with a volume of 12
50 SOUND 0,172 ' 8 least significant bits of frequency on channel A
60 SOUND 1,1 ' 4 most significant bits of frequency on channel A
70 SOUND 8,12 ' Sets the volume to 12 on channel A
80 SOUND 7,190 ' Activates the sound generator on channel A
90 PRINT "...call BEEP to stop playing."
