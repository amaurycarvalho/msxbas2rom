10 COLOR 15,1,4: SCREEN 5
20 LINE (40,40)-(215,151),7,BF
30 FOR I=0 TO 100
40 A=INT(RND(1)*173)+41
50 B=INT(RND(1)*109)+41
60 PRESET(A,B),,TPSET
80 NEXT I
90 GOTO 90
