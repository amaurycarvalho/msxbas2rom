' ---------------------
' TILE FLIP / ROTATE TEST
' ---------------------

10 SCREEN 2
20 SET TILE ON

' define a tile pattern (L-shape at tile 128)
30 SET TILE PATTERN 126, (128, 128, 128, 240, 0, 0, 0, 0)
31 FOR Y% = 0 TO 24
32   PUT TILE 126, ( 0, Y%)
33   PUT TILE 126, (31, Y%)
34 NEXT

' display tiles
40 LOCATE 10, 10 : PRINT "A B C D E F G H"
41 GOSUB 300

' flip horizontal (dir=0)
100 SET TILE FLIP 66, 0
101 GOSUB 300

' flip vertical (dir=1)
110 SET TILE FLIP 65, 1
111 GOSUB 300

' flip both (dir=2)
120 SET TILE FLIP 67, 2
121 GOSUB 300

' rotate left (dir=0)
130 SET TILE ROTATE 68, 0
131 GOSUB 300

' rotate right (dir=1)
140 SET TILE ROTATE 69, 1
141 GOSUB 300

' rotate 180 degrees (dir=2)
150 SET TILE ROTATE 70, 2
151 GOSUB 300

' same operations on specific bank
160 SET TILE FLIP 126, 0, 1
170 SET TILE ROTATE 126, 1, 2
171 GOSUB 300

200 END

300 A$ = INPUT$(1)
301 RETURN
