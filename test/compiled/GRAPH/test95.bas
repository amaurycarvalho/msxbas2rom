10 SCREEN 0
20 B% = BASE(0)
30 A% = HEAP()
40 S% = 40 * 24
50 CMD VRAMTORAM B%, A%, S%
60 FOR I% = A% TO A%+S%-1
70    POKE I%, 46
80 NEXT
90 CMD RAMTOVRAM A%, B%, S%
100 PRINT FRE()