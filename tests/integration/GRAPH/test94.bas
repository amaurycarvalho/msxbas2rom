10 DIM BA%(15), BB%(3,3), BC%(3,3)          ' sprite pattern memory buffers (16 integers = 32 bytes)
11 SCREEN 2, 2

' fill sprite pattern memory buffer A
' sprite pattern (16x16):  
'     low  BA%(0)  low  BA%(8)
'     high BA%(0)  high BA%(8)
'     low  BA%(1)  low  BA%(9)
'     high BA%(1)  high BA%(9)
'     low  BA%(2)  low  BA%(10)
'     high BA%(2)  high BA%(10)
'     low  BA%(3)  low  BA%(11)
'     high BA%(3)  high BA%(11)
'     low  BA%(4)  low  BA%(12)
'     high BA%(4)  high BA%(12)
'     low  BA%(5)  low  BA%(13)
'     high BA%(5)  high BA%(13)
'     low  BA%(6)  low  BA%(14)
'     high BA%(6)  high BA%(14)
'     low  BA%(7)  low  BA%(15)
'     high BA%(7)  high BA%(15)
20 FOR I% = 0 TO 15
21   BA%(I%)   = &hFF00
22 NEXT

' copy memory buffer A to sprite pattern #0 and show it
30 SET SPRITE PATTERN 0, BA%
31 PUT SPRITE 0,(50,100),15,0

' copy sprite pattern #0 to buffer B
40 GET SPRITE PATTERN 0, BB%

' modify memory buffer B content and fill buffer C
' sprite pattern (16x16):  
'     low  BB%(0,0)  low  BB%(0,2)
'     high BB%(0,0)  high BB%(0,2)
'     low  BB%(1,0)  low  BB%(1,2)
'     high BB%(1,0)  high BB%(1,2)
'     low  BB%(2,0)  low  BB%(2,2)
'     high BB%(2,0)  high BB%(2,2)
'     low  BB%(3,0)  low  BB%(3,2)
'     high BB%(3,0)  high BB%(3,2)
'     low  BB%(0,1)  low  BB%(0,3)
'     high BB%(0,1)  high BB%(0,3)
'     low  BB%(1,1)  low  BB%(1,3)
'     high BB%(1,1)  high BB%(1,3)
'     low  BB%(2,1)  low  BB%(2,3)
'     high BB%(2,1)  high BB%(2,3)
'     low  BB%(3,1)  low  BB%(3,3)
'     high BB%(3,1)  high BB%(3,3)
51 FOR I% = 0 TO 3
52   BB%(1,I%) = &h55AA
53   BB%(3,I%) = &h55AA
54   BC%(0,I%) = &h55AA
55   BC%(2,I%) = &h55AA
56 NEXT

' copy buffer B to sprite pattern #1 and show it
60 SET SPRITE PATTERN 1, BB%
61 PUT SPRITE 1,(100,100),15,1

' copy buffer C to sprite pattern #2 and show it
70 SET SPRITE PATTERN 2, BC%
71 PUT SPRITE 2,(150,100),15,2

80 A$ = INPUT$(1)
81 SCREEN 0
82 END

