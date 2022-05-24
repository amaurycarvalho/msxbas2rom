'====================================================
' GAME DEMONSTRATION 1 (FOR MSX 1)
' By: Amaury Carvalho, 2022
'====================================================
' How to compile:
'   msxbas2rom -c gd1.bas
' Links:
'   https://launchpad.net/msxbas2rom
'   https://www.julien-nevo.com/arkostracker/
'   https://launchpad.net/nmsxtiles
'   http://msx.jannone.org/tinysprite/tinysprite.html
'====================================================

FILE "gd1.akm"        ' Exported from Arkos Tracker 2
FILE "gd1.akx"        ' Exported from Arkos Tracker 2
FILE "gd1.chr.plet5"  ' Exported from nMSXTiles
FILE "gd1.clr.plet5"  ' Exported from nMSXTiles
FILE "gd1.spr"        ' Exported from TinySprite
FILE "gd1_0_0.plet5"  ' Exported from nMSXTiles

10 SCREEN 2, 2, 0
11 COLOR 15, 0, 0 

20 CMD DISSCR         ' Disable screen
21 CMD PLYLOAD 0, 1   ' Load music and sound effect resource
22 CMD WRTCHR 2       ' Load tiles patterns resource
23 CMD WRTCLR 3       ' Load tiles colors resource
24 CMD WRTSPR 4       ' Load sprite resource
25 CMD WRTSCR 5       ' Load screen resource
26 CMD SETFNT 0       ' Load system default font

30 CMD PLYSONG 0      ' Load game song theme (Bach Invention 14)
31 CMD PLYPLAY        ' Play theme

40 LOCATE 00, 00 : PRINT "GAME DEMONSTRATION 1";
41 LOCATE 22, 00 : PRINT "HITS:";
42 CMD ENASCR         ' Enable screen

'==== PLAYER START POSITION AND SCORE
50 PX% = 8
51 PY% = 24
52 PP% = 0

'==== SHOW SPIDER
60 SX% = RND(1) * 100 MOD 192 + 32
61 SY% = RND(1) * 100 MOD 128 + 32
62 PUT SPRITE 1, (SX%, SY%), 8

'==== SHOW PLAYER
70 PUT SPRITE 0, (PX%, PY%), 15, 0

'==== GET PLAYER INPUT
80 PS% = STICK(0) OR STICK(1)
81 PB% = STRIG(0) OR STRIG(1)

'==== CHECK FOR TRIGGER
90 IF PB% = 0 THEN 200
91     PUT SPRITE 0,,,2                   ' Change player's sprite pattern 
92     IF COLLISION(0, 1) < 0 THEN 110    ' Sprite 0 not collided with sprite 1?

'==== PLAYER HIT THE SPIDER
100 CMD PLYSOUND 2                        ' Play sound effect from instrument 2
101 PP% = PP% + 1
102 LOCATE 28, 00 : PRINT PP%;
103 PB% = STRIG(0) OR STRIG(1) 
104 IF PB% <> 0 THEN 103
105 GOTO 60

'==== PLAYER MISS THE SPIDER
110 CMD PLYSOUND 3                        ' Play sound effect from instrument 3
111 PB% = STRIG(0) OR STRIG(1) 
112 IF PB% <> 0 THEN 111
113 GOTO 70

'==== CHECK FOR PLAYER MOVIMENT 
200 IF PS% = 0 THEN 80
201   DX% = 0 : DY% = 0

210   IF PS% = 1 THEN DY% = -1 : GOTO 220
211   IF PS% = 2 THEN DX% = 1 : DY% = -1 : GOTO 220
212   IF PS% = 3 THEN DX% = 1 : GOTO 220
213   IF PS% = 4 THEN DX% = 1 : DY% = 1 : GOTO 220
214   IF PS% = 5 THEN DY% = 1 : GOTO 220
215   IF PS% = 6 THEN DX% = -1 : DY% = 1 : GOTO 220
216   IF PS% = 7 THEN DX% = -1 : GOTO 220
217   IF PS% = 8 THEN DX% = -1 : DY% = -1 

220 DX% = DX% + PX%
221 DY% = DY% + PY%
222 IF DX% < 8 OR DX% > 232 OR DY% < 24 OR DY% > 160 THEN 80

230 PX% = DX%
231 PY% = DY%
232 GOTO 70



