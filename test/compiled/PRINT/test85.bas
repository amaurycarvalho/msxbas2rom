10 CLS

20 A% = 7 : B% = 2 
30 C# = 7 : D# = 2
40 E! = 7 : F! = 2
45 G$ = "TESTE" : PRINT G$

50 PRINT A% \ B%; TAB(4); A% \ D#, A% \ F!
60 PRINT C# \ B%; TAB(4); C# \ D#, C# \ F!
70 PRINT E! \ B%; TAB(4); E! \ D#, E! \ F!

80  PRINT A% MOD B%, A% MOD D#, A% MOD F!
90  PRINT C# MOD B%, C# MOD D#, C# MOD F!
100 PRINT E! MOD B%, E! MOD D#, E! MOD F!

110 PRINT E! + C#, E! + A%, C# + A%

120 E! = 54% : PRINT USING "########";E!
130 E! = 54321# : PRINT USING "########";E!
140 E! = 87654321! : PRINT USING "########";E!

150 PRINT &HC000

160 END

170 RESUME




