10 DIM A%(10)
20 N% = 1
30 S$ = "A TEST"
40 POKE VARPTR(A%(0)), 5
50 POKE VARPTR(N%)+1, 1
60 POKE VARPTR(S$)+1, 66
70 PRINT A%(0), N%
80 PRINT S$
90 PRINT PEEK(VARPTR(S$))




