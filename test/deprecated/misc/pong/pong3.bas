INCLUDE "pong.chr.bin.plet5" 
INCLUDE "pong.clr.bin.plet5" 
INCLUDE "pong.spr.bin.plet5" 
INCLUDE "pong_splash.scr.bin.plet5" 
INCLUDE "pong_field.scr.bin.plet5" 
1 defint a-z : dim di(20,34)
10 screen1,2:width32:color15,1,1:keyoff
20 CALL WRTVRAM 0, &h0000  ' PATTERN GENERATOR TABLE (SCREEN 1)
30 CALL WRTVRAM 1, &h2000  ' PATTERN COLOR TABLE (SCREEN 1)
40 CALL WRTVRAM 2, &h3800  ' SPRITE PATTERNS TABLE (SCREEN 1)
300 rem for j=0 to 12:for i=0 to 33:reada:di(j,i)=a:nexti,j
440 cls
450 CALL WRTVRAM 3, &h1800  ' PATTERN LAYOUT TABLE - SPLASH SCREEN (SCREEN 1)
460 for i=0 to 6000:next:cls
461 cls:p1=0:p2=0
500 CALL WRTVRAM 4, &h1800  ' PATTERN LAYOUT TABLE - FIELD SCREEN (SCREEN 1)
600 rem for i=0 to 16:vpoke&h1800+di(p1,i*2)+18*32,di(p1,i*2+1):next
601 rem for i=0 to 16:vpoke&h1800+di(p2,i*2)+3*32+26,di(p2,i*2+1):next
650 x1=45:y1=130:x2=195:y2=65:x3=120:y3=88:v=3:xx=0:yy=v
700 putsprite 0,(x1,y1),15,0:putsprite 1,(x2,y2),15,0:putsprite 2,(x3,y3),15,1
710 j=stick(0)
711 if j=3 and x1<89 then x1=x1+2:y1=y1+1
712 if j=7 and x1>11 then x1=x1-2:y1=y1-1
720 j=stick(1)
721 if j=3 and x2<229 then x2=x2+2:y2=y2+1
722 if j=7 and x2>151 then x2=x2-2:y2=y2-1
800 x3=x3+xx:y3=y3+yy
801 b1=-x3/2+198:b2=x3/2+114:b3=-x3/2+112:b4=x3/2-45:b5=b2-15:b6=b4+15
811 if y3>b1 and yy=v then xx=-v:yy=0
812 if y3>b2 then gosub1000
813 if y3<b3 and yy=-v then xx=v:yy=0
814 if y3>b1 and xx=v then xx=0:yy=-v
815 if y3<b4 then gosub1200
816 if y3<b3 and xx=-v then xx=0:yy=v
817 if y3<b6 and y3>b6-8 and x3>x2-10 and x3<x2+10 and yy=-v then xx=-v:yy=0
818 if y3<b6 and y3>b6-8 and x3>x2-10 and x3<x2+10 and xx=v then xx=0:yy=v
819 if y3>b5 and y3<b5+8 and x3>x1-10 and x3<x1+10 and yy=v then xx=v:yy=0
820 if y3>b5 and y3<b5+8 and x3>x1-10 and x3<x1+10 and xx=-v then xx=0:yy=-v
890 if p1=9 or p2=9 then for i=0 to 5000:next:goto 440
900 goto 700
1000 x3=120:y3=88:xx=0:yy=-v
1001 p1=p1+1
1002 rem for i=0 to 16:vpoke&h1800+di(p1,i*2)+18*32,di(p1,i*2+1):nexti
1003 return
1200 x3=120:y3=88:xx=0:yy=v
1201 p2=p2+1
1202 rem for i=0 to 16:vpoke&h1800+di(p2,i*2)+3*32+26,di(p2,i*2+1):nexti
1203 return


