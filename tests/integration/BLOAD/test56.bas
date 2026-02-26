10 input "save or load";a$
20 if a$="L" then 80
30 screen 2
40 circle(100,100),80
50 rem bsave "screen.sc2",&H0,&H3FFF,s
60 screen 0
70 end
80 screen 2
90 rem bload "screen.sc2",s
100 a$=input$(1)
110 screen 0

