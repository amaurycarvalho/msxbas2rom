' LOADING RESOURCE TO RAM TEST
' HOW TO COMPILE:
'   pletter string.txt
'   pasmo string.asm string.bin
'   msxbas2rom -c rsc1.bas

FILE "string.txt.plet5"
FILE "string.bin"

10 A$ = SPACE$(100)                     ' string buffer
20 CMD RSCTORAM 0, VARPTR(A$)+1, 1      ' load resource (compressed by pletter) to string buffer
30 PRINT A$                             ' print string
35 PRINT FRE()                          ' print free memory size

40 CMD RSCTORAM 1, HEAP()               ' load resource (uncompressed) to top of RAM
50 DEF USR = HEAP()                     ' point USR to assembly routine
60 B% = USR(VARPTR(A$))                 ' call assembly routine to print string


