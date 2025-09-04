01 dim ma%(4,2)
02 DATA "AA", "BB", 6, 7

10 MA%(4,2) = 10
11 A$ = "TEST"
12 PRINT A$, ma%(4,2)

20 X% = 7 : Y%= 8 : PRINT X%, Y%
21 K% = 7 * 8 : PRINT K%,
22 K% = 7 * Y% : PRINT K%,
23 K% = X% * 8 : PRINT K%,
24 K% = X% * Y% : PRINT K%

30 K% = 7 + 8 : PRINT K%,
31 K% = 7 + Y% : PRINT K%,
32 K% = X% + 8 : PRINT K%,
33 K% = X% + Y% : PRINT K%

40 K% = 7 - 8 : PRINT K%,
41 K% = 7 - Y% : PRINT K%,
42 K% = X% - 8 : PRINT K%,
43 K% = X% - Y% : PRINT K%

50 X% = 8 : Y%= 4 : PRINT X%, Y%
51 K% = 8 / 4 : PRINT K%,
52 K% = 8 / Y% : PRINT K%,
53 K% = X% / 4 : PRINT K%,
54 K% = X% / Y% : PRINT K%

60 READ A$ : PRINT A$,
61 READ A$ : PRINT A$,
62 READ A$ : PRINT A$,
63 READ A$ : PRINT A$

70 END


