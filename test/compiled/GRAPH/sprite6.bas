FILE "sprite1.spr"

10 SCREEN 2, 2, 0
11 SPRITE LOAD 0                ' load sprites

20 GOSUB 100                    ' show sprites
21 D% = 0 : GOSUB 300           ' flip horizontal
22 D% = 1 : GOSUB 300           ' flip vertical
23 D% = 2 : GOSUB 300           ' flip both

30 D% = 0 : GOSUB 400           ' rotate left
31 D% = 1 : GOSUB 400           ' rotate right
32 D% = 2 : GOSUB 400           ' rotate 180 degrees

40 SCREEN 0
41 END

'--- SHOW SPRITES
100 COLOR 15,4,0
101 CLS
102 PUT SPRITE 0, (100,100)
103 PUT SPRITE 1, (100,100)
104 PUT SPRITE 2, (100,100)

'--- WAIT
200 A$ = INPUT$(1)
201 RETURN

'--- FLIP SPRITES
300 SET SPRITE FLIP 0, D%       
301 SET SPRITE FLIP 1, D%       
302 SET SPRITE FLIP 2, D%       
303 GOTO 200

'--- ROTATE SPRITES
400 SET SPRITE ROTATE 0, D%     
401 SET SPRITE ROTATE 1, D%     
402 SET SPRITE ROTATE 2, D%     
403 GOTO 200

