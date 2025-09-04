01 dim ma%(4,2)
02 DATA "AA", "BB", 6, 7
05 CMD PLYMUTE : MA%(4,2) = 5
10 X% = 20
20 X% = X%-19 
25 A$ = "TEST"
30 print x%, ma%(4,2), A$
31 READ A$ : PRINT A$
32 READ A$ : PRINT A$
33 READ A$ : PRINT A$
34 READ A$ : PRINT A$
40 END
50 cls


