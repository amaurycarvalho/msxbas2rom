01 DEFINT A-Z
05 DIM M(300) : T = 1
10 FOR P=2 TO 3
20 FOR I=0 TO 169 : NEXT I
30 FOR I=0 TO 10
40 IF I <> 6 THEN T = T+1
50 FOR K=T TO T+2 STEP 2 : FOR R=1 TO M(K) : NEXT R
60 NEXT K,I,P
