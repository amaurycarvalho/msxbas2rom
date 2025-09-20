' ------------------------------------
' DATA/IDATA MIXUP BUG WORKAROUND TEST
' ------------------------------------

10 DATA "TEST1", 123
11 DATA "TEST2", 321

20 IDATA 32767, 10000, 9999
21 IDATA 10, 20, 30
22 IDATA "TEST2"

30 CMD RESTORE 1        ' DATA resource (always the last one)
31 READ S$, A% : PRINT S$, A%

40 CMD RESTORE 0        ' IDATA resource (always previous the last one)
41 IREAD A%, B%, C% : PRINT A%, B%, C%
42 IREAD A%, B%, C% : PRINT A%, B%, C%
43 IREAD A% : PRINT A%

50 RESTORE 21
51 IREAD A%, B#, C : PRINT A%, B#, C

60 CMD RESTORE 1        ' DATA resource
61 RESTORE 11
62 READ S$, A% : PRINT S$, A%


