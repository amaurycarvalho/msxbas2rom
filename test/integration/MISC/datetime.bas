10 CLS
20 GET DATE y%, m%, d%, w%, f%
30 GET TIME hh%, mm%, ss%
40 PRINT y%, m%, d%, w%, f%
50 PRINT hh%, mm%, ss%
60 INPUT "Year"; y%
65 IF y% = 0 THEN END
70 INPUT "Hour"; hh%
80 SET DATE y%, m%, d%
90 SET TIME hh%, mm%, ss%
100 END

