FILE "Starlight.pt3.plet5"
FILE "sprite.bin.plet5"
10 color 15 : screen 2, 2, 0 : key off
11 CALL TURBO ON
12 dim s!(80,2), i!(80,1) 
13 dim c(80)
14 v!=2.0
15 q=20 : x% = 100 : y% = 150
20 CMD PT3LOAD 0
21 CMD PT3PLAY 
22 CMD WRTSPRPAT 1
30 t=(t+1)mod2:ti=(ti+1)mod4 
31 if ti = 0 then line(0,0)-(100,75),0,BF
32 if ti = 1 then line(100,0)-(250,75),0,BF
33 if ti = 2 then line(100,75)-(250,200),0,BF
34 if ti = 3 then line(0,75)-(100,200),0,BF
41 for e= 0 to q
42 if s!(e,2)=0 then s!(e,0)=rnd(1)*256:s!(e,1)=rnd(1)*212:c(e)=rnd(1)*255+1:dx=s!(e,0)-128:dy=s!(e,1)-106:dc=sqr(dx^2+dy^2):i!(e,0)=dx/dc:i!(e,1)=dy/dc:s!(e,2)=1
43 pset(s!(e,0),s!(e,1)),c(e): s!(e,0)=s!(e,0)+( i!(e,0)*(s!(e,2)/10) ):s!(e,1)=s!(e,1)+(i!(e,1)*(s!(e,2)/10))
44 if s!(e,0)<0 or s!(e,0)>255 or s!(e,1)<0 or s!(e,1)>211 then s!(e,2)=0 else line-(s!(e,0),s!(e,1)),c(e):s!(e,2)=s!(e,2)*v!
50 PUT SPRITE$ 0, (x%, y%), 15 
60 a% = stick(0) or stick(1)
61 if a% = 7 then x% = x% - 1 : if x% < 10 then x% = 10
62 if a% = 3 then x% = x% + 1 : if x% > 200 then x% = 200
63 if a% = 1 then y% = y% - 1 : if y% < 10 then y% = 10
64 if a% = 5 then y% = y% + 1 : if y% > 200 then y% = 200
70 next e
80 goto 30

