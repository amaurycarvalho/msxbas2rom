' STRING$ function and concatenation testing

10 CLS

20 ? "HELLO"+" "+"WORLD"+"!!!"

30 ? STRING$(5,65)
31 ? STRING$(6,66)
32 ? STRING$(5,65)+STRING$(6,66)+"!!!"

40 ? STRING$(5,65);STRING$(6,66)
41 ? STRING$(5,65),STRING$(6,66)

50 X% = 5 : Y% = 1
51 ? STRING$(X%,65)+STRING$(6-Y%,66)
52 IF X% > -1 THEN ? STRING$(X%,65)+STRING$(6-Y%,66)
53 IF X% > -1 THEN LOCATE 2,10 : ? STRING$(X%,65)+STRING$(6-Y%,66)
