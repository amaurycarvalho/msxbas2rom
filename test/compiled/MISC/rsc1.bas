' RESOURCE LOADING TO RAM TEST

FILE "string.txt.plet5"
FILE "string.bin"

10 A$ = SPACE$(100)                     ' string buffer
20 CMD RSCTORAM 0, VARPTR(A$)+1, 1      ' resource compressed by pletter
30 PRINT A$

40 CMD RSCTORAM 1, HEAP()               ' resource uncompressed
50 DEF USR = HEAP()                     ' point to assembly routine
60 B% = USR(VARPTR(A$))                 ' call assembly routine


