10 DIM PB%(3,3)         ' sprite pattern memory buffer (16 integers = 32 bytes)
11 DIM CB%(7)           ' sprite color memory buffer (8 integers = 16 bytes)

' fill sprite pattern memory buffer
' sprite pattern (16x16):  
'     low  PB%(0,0)  low  PB%(0,2)
'     high PB%(0,0)  high PB%(0,2)
'     low  PB%(1,0)  low  PB%(1,2)
'     high PB%(1,0)  high PB%(1,2)
'     low  PB%(2,0)  low  PB%(2,2)
'     high PB%(2,0)  high PB%(2,2)
'     low  PB%(3,0)  low  PB%(3,2)
'     high PB%(3,0)  high PB%(3,2)
'     low  PB%(0,1)  low  PB%(0,3)
'     high PB%(0,1)  high PB%(0,3)
'     low  PB%(1,1)  low  PB%(1,3)
'     high PB%(1,1)  high PB%(1,3)
'     low  PB%(2,1)  low  PB%(2,3)
'     high PB%(2,1)  high PB%(2,3)
'     low  PB%(3,1)  low  PB%(3,3)
'     high PB%(3,1)  high PB%(3,3)
20 FOR I% = 0 TO 3
21   PB%(1,I%) = &h55AA
22   PB%(3,I%) = &h55AA
23 NEXT

' fill sprite color memory buffer
' sprite color (16x16):  
'     low  CB%(0)
'     high CB%(0)
'     low  CB%(1)
'     high CB%(1)
'     low  CB%(2)
'     high CB%(2)
'     low  CB%(3)
'     high CB%(3)
'     low  CB%(4)
'     high CB%(4)
'     low  CB%(5)
'     high CB%(5)
'     low  CB%(6)
'     high CB%(6)
'     low  CB%(7)
'     high CB%(7)
30 FOR I% = 0 TO 7
31   CB%(I%) = &h080F       ' first line color = 15, next line color = 8
32 NEXT

40 SCREEN 2, 2      ' MSX1
41 GOSUB 100

50 SCREEN 4, 2      ' MSX2
51 GOSUB 100

60 SCREEN 0
61 END

' copy buffers to sprite pattern #0 and show it
100 SET SPRITE PATTERN 0, PB%
101 SET SPRITE COLOR 0, CB%
102 PUT SPRITE 0,(100,100),,0
103 A$ = INPUT$(1)
104 RETURN

