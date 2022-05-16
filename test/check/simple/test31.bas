10 CLS
20 A% = 1 : B% = 2
21 K% = A% + B% + A% : PRINT K%, 
22 K% = K% + 1 + 1 : PRINT K%,
23 K% = 1 + K% : PRINT K%, 
24 K% = K% + A% + 3 : PRINT K%,
25 K% = K% - 1 : PRINT K%, 
26 K% = K% - 2 : PRINT K%,
27 K% = K% - 3 : PRINT K%,
28 K% = K% - 4 : PRINT K%,
29 K% = K% - 5 : PRINT K%
30 SWAP A%, B%
40 PRINT A%, B%
50 C# = 1.5 : D# = 2.5
55 W# = C# + D# + C#
60 SWAP C#, D#
70 PRINT C#, D#, W#
80 E$ = "A" : F$ = "B"
90 SWAP E$, F$
100 PRINT E$, F$
110 PRINT "Press CAPS LOCK to stop"
120 WAIT 170,64,64  ' wait until CAPS LOCK is on
