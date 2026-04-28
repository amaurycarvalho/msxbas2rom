10 CLS
20 PRINT "Free memory: "; FRE(0) 
30 S% = DSKF(0)
40 IF S% >= 0 THEN PRINT "Free clusters in the std drive:"; S% ELSE PRINT "Please insert a new disk!"

