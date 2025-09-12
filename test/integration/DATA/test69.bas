FILE "test69.csv"
FILE "test70.txt"

10 CMD RESTORE 0

20 READ R$, N%
30 IF R$="*" THEN 60
40 PRINT R$, N%
50 GOTO 20

60 READ V# : PRINT N%, V# 

70 RESTORE
71 READ R$ : PRINT R$

80 RESTORE 3
90 READ N% : PRINT N%

100 CMD RESTORE 1
110 RESTORE 2 
111 READ R$ : PRINT R$

120 END

' PRINT RESULT:
' VALUE A  1
' VALUE B  2
' VALUE C  3
' VALUE D          4
' VALUE E          5
' VALUE F  6
'  32      .1415
' VALUE A
'  5
' incididunt ut labore et dolore magna

