FILE "EYE1.S12"
FILE "EYE2.S12"

10 SCREEN 12
20 SCREEN LOAD 0 
21 COPY (0,0)-(255,211),0 TO (0,0),1
22 SCREEN LOAD 1 
30 VDP(1)=100   ' page wave
'31 VDP(1)=96    ' page swap
33 VDP(14)=17   ' speed=fast
'34 VDP(14)=28   ' speed=slow
'35 VDP(14)=0     ' stop
'36 VDP(10)=VDP(10) OR 4 ' set bit 3 to 1 (blink mode ON)
40 SET PAGE 1,1
50 IF STRIG(0) = 0 THEN 50
60 SCREEN 0

