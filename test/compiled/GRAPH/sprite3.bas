FILE "sprite1.spr"
FILE "sprite2.spr"

10 SCREEN 2, 2, 0
20 SPRITE LOAD 0
30 GOSUB 100

50 SCREEN 7, 2, 0
60 SPRITE LOAD 1
70 GOSUB 100

90 SCREEN 0
91 END

100 COLOR 15,4,0
101 CLS
102 PUT SPRITE 0, (100,100)
103 PUT SPRITE 1, (100,100)
104 PUT SPRITE 2, (100,100)
105 A$ = INPUT$(1)
106 RETURN

