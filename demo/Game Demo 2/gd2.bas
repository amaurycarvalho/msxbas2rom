'====================================================
' GAME DEMONSTRATION 2 (FOR MSX 2/2+, MEGAROM)
' By: Amaury Carvalho, 2022
'====================================================
' How to compile (megarom):
'   msxbas2rom -c -x gd2.bas
' Links:
'   https://github.com/amaurycarvalho/msxbas2rom/
'   https://www.julien-nevo.com/arkostracker/
'   http://msx.jannone.org/conv/
'   http://msx.jannone.org/tinysprite/tinysprite.html
'====================================================

FILE "gd2.akm"        ' 0 - Music (exported from Arkos Tracker 2)
FILE "gd2.akx"        ' 1 - Sound Effects (exported from Arkos Tracker 2)
FILE "gd2_1.spr"      ' 2 - Sprites for MSX 1 (exported from TinySprite)
FILE "gd2_2.spr"      ' 3 - Sprites for MSX 2 (exported from TinySprite)
FILE "SCREEN.SC2"     ' 4 - Background for Screen 2 (exported from MSX Screen Converter)
FILE "SCREEN.SC8"     ' 5 - Background for Screen 8 (exported from MSX Screen Converter)
FILE "SCREEN.S12"     ' 6 - Background for Screen 12 (exported from MSX Screen Converter)

'==== GAME INITIALIZATION
10 DIM AX%(5), AY%(5), AS%(5), AIX%(5), AIY%(5)

20 IF MSX() = 0 THEN SN% = 2 : SR% = 4 : SS% = 2 : GOTO 30
21 IF MSX() = 1 THEN SN% = 8 : SR% = 5 : SS% = 3 : GOTO 30
22 SN% = 12 : SR% = 6 : SS% = 3

'==== SCREEN INITIALIZATION
30 CMD DISSCR         ' Disable screen
31 SCREEN SN%, 2, 0   ' Change screeen mode
'32 CMD PLYLOAD 0, 1   ' Load music and sound effect resource
'33 SPRITE LOAD SS%    ' Load sprite resource
34 SCREEN LOAD SR%    ' Load screen resource
'35 CMD PLYSONG 0      ' Load game song theme (Gustav Holst - The Planets - Mars)
'36 CMD PLYPLAY        ' Play theme
37 CMD ENASCR         ' Enable screen
38 GOTO 38

'==== PLAYER AND ASTEROIDS START POSITION 
40 PX% = 120
41 PY% = 100
42 FOR I% = 0 TO 5
43    GOSUB 1020 : AX%(I%) = R% MOD 192 + 32
44    GOSUB 1020 : AY%(I%) = R% MOD 128 + 32
45    AS%(I%) = 2
46    AIX%(I%) = 1 : AIY%(I%) = 1
47 NEXT

50 GOSUB 240   ' SHOW PLAYER
51 GOSUB 310   ' SHOW ASTEROIDS

'==== MAIN LOOP
60 GOSUB 1000  ' GET PLAYER INPUT
61 GOSUB 200   ' MOVE PLAYER
62 GOSUB 300   ' MOVE ASTEROIDS
63 GOSUB 400   ' MOVE BULLETS
64 GOSUB 500   ' CHECK FOR TRIGGER
65 GOTO 60

'==== MOVE PLAYER
200 IF PS% = 0 THEN RETURN
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
222 IF DX% < 8 OR DX% > 232 OR DY% < 24 OR DY% > 160 THEN RETURN

230 PX% = DX%
231 PY% = DY%

'==== SHOW PLAYER
240 PUT SPRITE 0, (PX%, PY%), 15, 0
241 PUT SPRITE 1, (PX%, PY%),  8, 1
242 RETURN

'==== MOVE ASTEROIDS
300 FOR I% = 0 TO 5
301   AX%(I%) = AX%(I%) + AIX%(I%) : IF AX%(I%) < 10 OR AX%(I%) > 200 THEN AIX%(I%) = -AIX%(I%)
302   AY%(I%) = AY%(I%) + AIY%(I%) : IF AY%(I%) < 10 OR AY%(I%) > 190 THEN AIY%(I%) = -AIY%(I%)
303 NEXT

'==== SHOW ASTEROIDS
310 FOR I% = 0 TO 5
311    PUT SPRITE 2+I%, (AX%(I%), AY%(I%)), 14, 2+I%
312 NEXT
313 RETURN

'==== MOVE BULLETS 
400 RETURN

'==== CHECK FOR TRIGGER
500 IF PB% = 0 THEN RETURN
501    RETURN

'==== BULLET HIT AN ASTEROID
510 CMD PLYSOUND 2                        ' Play sound effect from instrument 2
511 RETURN

'==== PLAYER HIT AN ASTEROID
520 CMD PLYSOUND 3                        ' Play sound effect from instrument 3
521 RETURN

'==== GET PLAYER INPUT
1000 PS% = STICK(0) OR STICK(1)
1001 PB% = STRIG(0) OR STRIG(1)
1002 RETURN

'==== WAIT FOR TRIGGER RELEASE
1010 PB% = STRIG(0) OR STRIG(1) 
1011 IF PB% <> 0 THEN 1010
1012 RETURN

'==== GET RANDOM NUMBER
1020 R% = RND(1) * 100
1021 RETURN





