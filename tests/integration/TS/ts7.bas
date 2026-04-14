' ------------------
' TINY SPRITE TEST
' 64 PATTERNS LIMIT
' ------------------

FILE "sprite4.spr"   ' resource 0: msx2 sprite set

10 SCREEN 5, 2, 0    
20 SPRITE LOAD 0     ' load resource 0 (msx1 sprite set)

30 PUT SPRITE 0,(100, 20)
31 PUT SPRITE 1,(100, 60)
32 PUT SPRITE 2,(100, 60)
33 PUT SPRITE 3,(100,100)
34 PUT SPRITE 4,(100,100)
35 PUT SPRITE 5,(100,140)
36 PUT SPRITE 6,(100,140)

40 A$ = INPUT$(1)

50 SCREEN 0          
51 END

