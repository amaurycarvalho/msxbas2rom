10 defint a-z
20 screen 8, 1

21 CMD TURBO 1

22 x=0 : y = 0
25 for i=0 to 255
30 line (x, y)-(x+13, y+13), i, BF 'pset(10, 10), 15
31 x = x + 14
32 if x > 255 then x = 0 : y = y + 15
35 next i

40 s$ = ""
50 for i=0 to 7 : read a : s$ = s$ + chr$(a) : next i
60 sprite$(0) = s$

78 for j = 0 to 15
79 x=0 : y=0
80 for i=0 to 31
90 put sprite i, (x + j, y + (j * 2)), i + j, 0
91 x = x + 32
92 if x > 255 then x = 0 : y = y + 48
100 next i
110 next j

200 goto 78

1000 data 24,60,126,255,36,36,66,129
