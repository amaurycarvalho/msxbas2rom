' ------------------
' TINY SPRITE TEST
' 64 PATTERNS LIMIT
' ------------------

FILE "sprite4.spr"   ' resource 0: msx1 sprite set

10 SCREEN 2, 2, 0    ' screen mode 2 (msx1)
20 SPRITE LOAD 0     ' load resource 0 (msx1 sprite set)
' 25 PUT SPRITE 0,(100,100),15
30 A$ = INPUT$(1)
40 SCREEN 0          
50 END

