10 CLS
20 PRINT "<<<< STRING FUNCTIONS >>>>"
30 B$ = "THIS" + " IS "
40 C$ = "A TEST"
50 D$ = B$ + C$
60 PRINT "TEXT 1: "; B$
70 PRINT "TEXT 2: "; C$
80 PRINT "TEXT 1 + TEXT 2: "
85 PRINT "=>";D$
90 PRINT "CHR$ 65,66,67: ";CHR$(65);CHR$(66);CHR$(67)
95 A$ = "ABCDEF"
100 PRINT "ASC(ABCDEF) = "; ASC(A$)
110 PRINT "LEN(ABCDEF) = "; LEN(A$)
120 PRINT "PRESS A KEY..."
130 B$ = INPUT$(1)
140 PRINT "ASC "; B$; " = "; ASC(B$)
150 PRINT "LEFT(ABCDEF,2) = "; LEFT$(A$, 2)
160 PRINT "RIGHT(ABCDEF,2) = "; RIGHT$(A$, 2)
170 PRINT "MID(ABCDEF,3) = "; MID$(A$, 3)
180 PRINT "MID(ABCDEF,3,2) = "; MID$(A$, 3, 2)
190 PRINT "SPACE(5);ABCDEF ="; SPACE$(5); A$
200 PRINT "SPC(5);ABCDEF ="; SPC(5); A$
210 PRINT "TAB(1);ABCDEF ="; TAB(1); A$
220 PRINT "STRING(5,A) ="; STRING$(5,"A")
230 PRINT "OCT(9) = ";OCT$(9)
231 PRINT "HEX(32) = ";HEX$(32)
232 PRINT "BIN(5) = ";BIN$(5)
240 PRINT "STR(50) = "; STR$(50)
250 PRINT "INSTR(CDE,ABCDEF) = "; INSTR("CDE",A$)
260 PRINT "INSTR(4,CDE,ABCDEF) = "; INSTR(4,"CDE",A$)
270 MID$(A$,2) = "XY"
280 MID$(A$,5,1) = "KW"
290 PRINT "MID ASSIGN = "; A$


