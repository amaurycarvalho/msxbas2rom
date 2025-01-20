10 SCREEN 4
11 SET TILE ON
20 bload "teste.sc2",s
30 a$=input$(1)

' the line below works for screen4, but base(10) is meant for screen2
40 FOR I = 0 TO 767: VPOKE BASE(10)+I,255:NEXT I
45 a$=input$(1)

' this is the same as above, but replacing base(10) with (20) which is the
' correct for screen4, but does not work (screen does not change)
50 FOR I = 0 TO 767: VPOKE BASE(20)+I,1:NEXT I
55 a$=input$(1)

' put tile starts at somewhere in the middle of the first third of screen,
' not the top left corner
60 FOR I = 0 TO 767: PUT TILE 2, ( i mod 32, i \ 32 ):NEXT I
61 PUT TILE 3, (0, 0)
65 a$=input$(1)
