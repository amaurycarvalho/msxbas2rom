' Cadari Bit example 2
' CMD PAGE syntax:
'     CMD PAGE <mode: 0=swap, 1=wave>, <delay #1: 0-15, 0=stop>
'              [, <delay #2: 0-15, default=same as delay #1>]
' Compile: msxbas2rom -c -x cadbit2.bas

FILE "EYE1.S12"     ' 0
FILE "EYE2.S12"     ' 1

10 SCREEN 12

20 SCREEN LOAD 0 
21 COPY (0,0)-(255,211),0 TO (0,0),1
22 SCREEN LOAD 1 

30 CMD PAGE 0, 8     ' swap slow
31 A$ = INPUT$(1)

40 CMD PAGE 0, 1     ' swap fast
41 A$ = INPUT$(1)

50 CMD PAGE 1, 12, 1 ' wave slow
51 A$ = INPUT$(1)

60 CMD PAGE 1, 1     ' wave fast
61 A$ = INPUT$(1)
   
70 CMD PAGE 0, 0     ' stop the effect
71 A$ = INPUT$(1)

80 SCREEN 0

