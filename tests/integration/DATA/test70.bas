FILE "test70.txt"
FILE "test70.txt"
FILE "test70.txt"

10 CMD RESTORE 2

20 READ R$
30 IF R$="*" THEN 60
40 PRINT R$
50 GOTO 20

60 RESTORE
70 READ R$ : PRINT R$

80 RESTORE 160
90 READ R$ : PRINT R$

100 END

' PRINT RESULT:
' Quamobrem cum vitiorum ista vis non sit
'  ut causam
' alferant mortis voluntariae, perspicuum
'  est etiam
' stultorum qui iideni miseri sint offici
' um esse manere
' in vita, si sint in maiore parte earum
' rerura quas
' secundum naturam esse dicimus, Et quoni
' am exce-
' dens e vita et manens aeque miser est,
' nee diutumttas
' magis ei vitam fugiendam facit, non sin
' e causa dicitur
' iis qui pluribus naturalibus frui possi
' nt esse in vita
' manendum.
' Lorem ipsum dolor sit amet, consectetur
'
' manendum.

