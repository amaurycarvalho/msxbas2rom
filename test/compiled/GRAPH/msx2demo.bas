1 'save"msx2demo.asc",a          ' Modificado By Cadari 2021
9 CLEAR 1000,&HD800
100 OPEN "grp:" FOR OUTPUT AS #1
105 COLOR15,10,0:SCREEN7:COLOR=(10,0,0,0):DEFUSR=&H44:A=USR(0)
110 RESTORE110:K=15:GOSUB9100:DATA0,1,2,3,4,5,6,7,224,0,0,0,557,224,777
150 SET PAGE SP,1:COLOR 15,0:CLS
155 X=1:Y=0:C=6:D=1:A$="MSX 2.0":GOSUB5700:D=1:A$="Tecnologia em Inform�tica":GOSUB 5700
160 X=202:Y=86
165 DRAW"S4A0c9bm=X;,=Y;bR6D9"
170 A$="bm=X;,=T;m+74,-18 m+40,+9 m-70,+23 m-44,-14":FOR T=Y+4 TO Y+1 STEP -1:DRAW"C4XA$;":NEXTT:DRAW"C3XA$;":PAINT (X+10,Y),3,3
175 DRAW"C3XA$;":PAINT (X+10,Y),3,3
180 FOR I=1TO8:DRAW"bm=X;,=Y;BD2;":DRAW"C9br120bu9":DX=INT(I*70/8):DY=INT(I*23/8):DRAW"bm-=dx;,+=dy;d9":NEXT
185 SETPAGE 0,0:X=0:Y=0
190 AA=83:CH=3:CL=1:CV=1:CB=11
195 LINE (0,80+AA)-(511,80),CB:PAINT (0,211),CB,CB
200 SOUND 7,&B110111 :SOUND8,5
203 XR=RND(1)*511:YR=RND(1)*211:CR=RND(1)*11+5
204 SOUND6,8:FORI=1TO140:PSET(XR,YR),CR:NEXTI
206 M=4:JJ=16:W=8:W2=150
208 SOUND8,5
210 PSET(0,AA+80),15:S=5:FORI=1TOS:LINE -(I*512/S,80+AA-I*AA/S),15:NEXTI
212 COLOR=(10,7,7,7):COLOR=(11,7,7,7)
214 COLOR=(10,0,0,0):COLOR=(11,0,0,0)
216 LINE (0,79+AA)-(511,79),1
218 RC=RND(1)*11+2:SOUND 8,15:FORJ=0TO31-YY:SOUND6,J:SOUND8,15:LINE(0,80+AA)-(511,80),RC:FORK=0TOW:NEXTK:NEXTJ
220 LINE (0,AA+80)-(511,80),CH
222 FORJ=20TO31:SOUND6,J:SOUND8,15:FORK=0TOW:NEXTK:NEXTJ
224 FORI=1TO7:COLOR=(CB,0,0,I):FORK=1TO10:NEXTK:NEXT
226 FORI=1TO6:COLOR=(CB,0,0,7-I):FORK=1TO30:NEXTK:NEXT
250 I = 80:T=14
252 K=3:C=1:D=0
254 IF I+AA>212 THEN Y=(I+AA-212)*6
256 LINE (Y,I+AA)-(511,I),CL
258 SOUND 8,T:T=T-1
260 I=I+K:K=K+4:J=J+1
262 IF I+AA>212 THEN Y=(I+AA-212)*6
264 LINE (Y,I+AA)-(511,I),CL
266 PAINT (509,I-K+6),C,CL
268 D=(D+1)MOD8:C=D+1
270 IF I<212 THEN 254
272 X=80
274 FORI=0TO512STEP20
276 SX=I:SY=80+AA-I/6:DX=I*8-1750:DY=312+(250-I)/2:CO=CV
278 '
280 IF DY>212 THEN DX=(212-SY)*(DX-SX)/(DY-SY)+SX:DY=212
282 IF DX>511 THEN DY=(DY-SY)*(511-SX)/(DX-SX)+SY:DX=511
284 IF DX<0 THEN DY=(DY-SY)*(-SX)/(DX-SX)+SY:DX=0
286 LINE (SX,SY)-(DX,DY),CO
288 NEXTI
300 A$="GBDFACEGBE-GB-"
305 SOUND7,&B111000:PLAY"v12T199o2l1","v12T199o4l1","v12T199o3l3"
310 PLAY"GFEE-","GGGG",A$
315 PLAY"GFEE-","o5GGGG",A$
320 PLAY"GFEE-","o6go5go4go3g",A$
325 FORK=1TO50:NEXTK:COPY (200,60)-STEP(130,60),1 TO (200,60),0,TPSET
330 PLAY"GFEE-","o4ggg",A$
335 COPY (1,0)-(200,20),1 TO (50,30),0,TXOR
340 FORI=1TO7:COLOR=(12,I,0,0):FORK=0TO7:NEXTK:NEXTI:COLOR=(12,7,7,7):FORK=1TO70:NEXTK:FORI=7TO0STEP-.5:COLOR=(12,7,I-.5,I):FORK=0TO26:NEXTK:NEXTI
345 FORI=1TO20:COLOR=(RND(1)*3+13,RND(1)*8,RND(1)*8,RND(1)*8):NEXTI
350 FORP=1TO8:FORI=1TO8:COLOR=(I,0,0,8-I):NEXTI:FORI=1TO8:COLOR=(I,0,0,I-1):NEXTI:NEXTP
355 FORI=1TO7:COLOR=(10,I,0,0):FORK=1TO10:NEXTK:NEXTI:COLOR=(10,7,7,7):FORI=1TO7:COLOR=(10,7-I,0,0):FORK=1TO10:NEXTK:NEXTI
360 C=15:A$="":X=P+120:Y=Q+50:D=1:LL=0:XS=8:GOSUB5751:SETPAGESP,1:COLOR15,0:CLS:X=90:Y=140:C=1:S=12:GOSUB8000:FORI=1TO50:PSET(RND(1)*511,RND(1)*211),14:NEXTI
400 J=0:ONINTERVAL=6GOSUB401:SOUND 7,&B110111 :SOUND8,15:COLOR=(2,0,0,0):INTERVALON:COLOR=(2,0,0,0):SETPAGE1,1:COLOR15,0,0:GOTO 402
401 J=J+1:SOUND 6,J:SOUND8,15-J:IFJ=14THENINTERVALOFF:RETURN ELSE RETURN
402 CR=6
410 COLOR=(4,1,3,7)
430 X=180:Y=17:C=4:S=4
435 X=220:Y=35:C=4:S=12
440 SOUND 7,&B101010
450 FORI=1TO7:SOUND9,15:SOUND 6,I:COLOR=(1,I,0,0):FORK=1TO10:NEXTK:NEXTI
455 FORI=1TO3:SOUND 6,I+9:COLOR=(1,7-I,0,0):FORK=1TO10:NEXTK:NEXTI
460 FORI=13TO31:SOUND 6,I:FORK=1TO10:NEXTK:NEXTI:SOUND 7,&B111000
465 A$="t44v15s9m22000o1":PLAY A$,A$,A$:PLAY "C1","E1","G1"
470 FORI=1TO7:COLOR=(2,I,I,I):FORK=1TO1:NEXTK:NEXTI
475 FORK=1TO90:NEXTK
480 FORI=1TO7:COLOR=(2,7,7-I,7-I):FORK=1TO50:NEXTK:NEXTI
485 FORI=1TO3:COLOR=(2,7-I,0):FORK=1TO40:NEXTK:NEXTI
490 C=4:X=70:Y=150:A$="            O    S I S T E M A             ":D=1:GOSUB5700
495 IF PLAY(1) OR PLAY(2) OR PLAY(3) THEN 495
500 SETPAGE1,0
502 CC=1:CF=3:CS=4:C=5:CR=6:CN=7:CT=8:CK=9:CB=10:C1=11:C2=12:C3=13:CW=15
503 COLOR=(CB,0,0,2):COLOR 15,CB:CLS:COLOR15,CB,CB:SETPAGE0,0
506 RESTORE 506:K=13:GOSUB9100:DATA0,2,700,0,222,700,210,111,111,2,500,50,740
508 PSET(0,0),0,PSET
510 P=110:Q=80:X$="bm110,80c=c;"
512 DRAW"s4a0xx$; m+66,-26m+226,+24m-54,+32nd7m-240,-30d7m+248,+31m+4,+2d3m-44,+44m-6,+4m-8,+1m-258,-43m-4,-2u3m+60,-36m+8,-1"
514 DRAW"xx$;bm+292,-2 m+2,+1d34 m-90,+60m-6,+2 m-252,-42u2
516 DRAW"xx$;bm+286,+13 m-34,+20d2m+34,-20u2
518 PAINT (P+10,Q),CC,C
520 PAINT (P+10,Q+5),CC,C
522 PAINT (P+10,Q+50),CC,C
524 PAINT (P+290,Q+5),CC,C
526 DRAW"xx$; bm+80,+6 m+10,-4m+64,+8m-10,+4m-64,-8"
528 DRAW"xx$; bm+15,-2 m+6,-2m+16,+2m-6,+2m-16,-2"
530 DRAW"xx$; c=c1; bm+44,+0 r4d1l4
532 DRAW"xx$; c=c2; bm+52,+1 r4d1l4
534 DRAW"xx$; c=c3; bm+60,+2 r4d1l4
536 PAINT (P+20,Q-3),C,C
538 A$="m+72,+9m-8,+3m-72,-9m+9,-3"
540 B$="bm+15,+2nm-8,+3bm+15,+2nm-8,+3bm+15,+1nm-8,+3bm+15,+2nm-8,+3"
542 DRAW "xx$; bm+2,+15   xa$;"
544 PAINT (P+36,Q+21),CK,C
546 DRAW "xx$; bm+2,+15   xb$;"
548 DRAW "xx$; bm+110,+29 xa$;"
550 PAINT (P+143,Q+35),CK,C
552 DRAW "xx$; bm+110,+29 xb$;"
554 ' keyboard
556 DRAW "xx$; bm-10,+21 nm+184,+25 m-30,+18 m+185,+29 m+27,-22
558 DRAW "xx$; bm-17,+25 nm+184,+26
560 DRAW "xx$; bm-25,+30 nm+184,+26
562 DRAW "xx$; bm-33,+35 nm+184,+27
564 ' ctrl/spacebar
566 DRAW"xx$;bm+2,+23m-15,+21 m-9,+4 m+12,+2 nm+9,-4 m+114,+18 nm+9,-4 m+12,+2 m+43,-27
568 'cursors
570 DRAW "xx$;bm+158,+71 nm+18,-5 m+21,-18 nm+5,+8 m+35,+4 nm-19,+5 m-19,+19 nm-7,-8 m-36,-5
572 DRAW "xx$;bm+176,+66 m+7,-6 m+12,+1 m-7,+6 m-12,-1
574 'keys
576 PAINT(P+65,Q+34),CK,C
578 PAINT(P+65,Q+39),CK,C
580 PAINT(P+65,Q+46),CK,C
582 PAINT(P+65,Q+53),CK,C
584 PAINT(P+65,Q+59),CK,C
586 ' trademark
588 A$="m-4,+2":DRAW"xx$;c=ct; bm+216,-2 m+36,+4xa$;m-36,-4xa$;nm+9,+1xa$;nm+27,+3xa$;nm+27,+3xa$;nm+9,+1 g1 nm+10,+1
590 DRAW"bm-10,+5 c=cn; m+56,+7m+2,-1nm-56,-7 m+4,-2nm-16,-2 m+2,-1nm-16,-2 m+2,-1nm-8,-1
600 COLOR=(15,7,7,7):F=0:ON INTERVAL=20 GOSUB 601:INTERVALON:GOTO 602
601 F=1-F:COLOR=(CF,7,2*F,2*F):RETURN
602 XS=8:C=CW:Y=0:D=1:A$="     O MSX 2.0    ":GOSUB5720:D=1:A$="Eh um computador semi-PROFISSIONAL....":GOSUB 5720
604 SETPAGESP,1:COLOR15,0:CLS:CIRCLE(380,100),110,14:PAINT(0,0),4,14:SETPAGE0,0
606 RESTORE 610
608 C=CR:C2=CF
610 DATA-3,14,-20,40,295,-35,"etiqueta\MSX 2.0\  "
612 DATA20,-5,20,-20,-85,80,"192 Kb RAM\64 Kb ROM
614 DATA30,18,45,30,-100,-20,"253 caracteres\no teclado"
616 DATA10,18,-45,45,154,-55,"2 conectores\Interface universal"
618 DATA-2,0,-20,-20,264,80,"24 linhas\de 80 colunas de texto"
620 DATA10,-4,10,-30,30,96,"graficos em\256 cores"
622 DATA6,-2,-8,-32,170,110,"512x212\pixels graficos
624 DATA20,18,28,56,0,-50,"10 funcoes\programaveis"
626 DATA15,-3,30,-45,-36,113,"3 geradores\de voz e de sons"
630 FORK=1TO9
633 READA,B,I,J,X,Y,A$:X=X+P:Y=Y+Q
634 FOR T=15TOCSTEP-1:LINE(X+A,Y+B)-STEP(I,J),T:NEXT
635 IFK=4THENLINE(X+40,Y+18)-STEP(-8,24),C
636 GOSUB5750:NEXTK
650 SETPAGE0,1
658 RESTORE 660:C=15:X=40:Y=50:FORK=1TO4:READA$:D=1:GOSUB5700:NEXTK
660 DATAE DEPOIS MSX,"EH UM PADRAO MUNDIAL,",DE PROGRAMAS,PROFISSIONAIS DISPONIVEIS
666 SETPAGE0,0:COLOR15,0,CS:FORI=0TO106:COPY(0,I)-STEP(511,0),1 TO (0,I),0,TPSET:COPY(0,211-I)-STEP(511,0),1 TO (0,211-I),0,TPSET:NEXTI
700 '
751 GOSUB 9150
752 A$=STRING$(15,0):SPRITE$(0)=CHR$(240)+A$:SPRITE$(1)=STRING$(3,64)+A$:SPRITE$(2)=CHR$(&B1111111)+CHR$(&B111110)+CHR$(&B11100)+CHR$(&B1000)+A$
754 SETPAGESP,1:COLOR 1,15:CLS
756 P=16:Q=10
758 LINE (0,Q+16)-(511,0),14,BF:LINE (0,Q+16)-(511,0),1,B
760 CT=1:A$="TRATAMENTO DE TEXTOS MSX":X=256-8*LEN(A$)/2:PSET(X,2),0,TPSET:COLORCT,0:PRINT#1,A$:PSET(X-1,2),0,TPSET:PRINT#1,A$
762 FORJ=0 TO 480 STEP 6:LINE(P+J,Q+17)-STEP(0,-4),1:IFJMOD60=0THENLINE(P+J,Q+17)-STEP(0,-7),1:A$=STR$(J/6):C=1:X=J-2:Y=Q+2:IFJ=0THENA$=" "+A$:GOSUB5700ELSEGOSUB5700
764 NEXTJ
766 CT=1
770 FORI=30 TO 211 STEP 2:PSET (P+34,I),9:NEXT
780 COPY(P+34,30)-STEP(0,181) TO (P+450,30)
800 COLOR=(14,5,5,5):COLOR=(8,7,1,1):C2=11:COLOR=(C2,7,7,1)
801 SETPAGE0,0:D=1:XS=8:C=8:Y=50+6*9:A$="Tratamentos de Textos...":L=LEN(A$)*8:X=246-L/2:LINE (X-20,Y-10)-STEP(L+40,26),C2,BF:LINE-(X-20,Y-10),8,B:GOSUB5751:Y=Y-9:FORK=1TO400:NEXTK:COPY(X-20,Y-10)-STEP(L+40,26),0 TO (X-20,Y-10),1,TPSET
802 COLOR 0,15,15:FORI=0TO256STEP2:COPY(I,0)-STEP(1,211),1 TO (I,0),0:COPY(511-I,0)-STEP(1,211),1 TO (511-I,0),0:NEXTI:INTERVALOFF
803 PUTSPRITE3,(P/2+13,18),6,2:PUTSPRITE4,(P/2+221,18),6,2
804 FOR I=1TO7:COLOR=(C2,6+I/4,6+I/4,I):COLOR=(8,7,I,I):NEXTI:LINE(X-20,Y-10)-STEP(L+40,26),15,BF
805 RESTORE805:DATA"Sao Paulo 9 de Maio 1986",,,"      Cara  Angelica,",,,,"Este otimo tratamento de textos trabalhado em 80 colunas,","dispoe de um padrao internacional de caracteres,","e eh muito facil de ser utilizado.","   ",Abracos,"     Felipe
806 X=P+300:Y=Q+27+9:FORJ=1TO12
808 READA$:IFA$="" THEN A$=" "
810 XS=6:COLOR1,0
812 FORI=1TOLEN(A$):PSET(X,Y),0,TPSET:X=X+XS:A=ASC(MID$(A$,I,1))
814 FORK=1TOINT(RND(1)*1.4)*30:NEXTK:IF A<>32 THEN SOUND6,11:SOUND 7,&B110111:SOUND8,12
816 PRINT#1,CHR$(A)
818 SOUND8,0
820 PUT SPRITE 0,(X/2,Y+6),6,0
822 PUT SPRITE 1,(X/2, 21),6,1
824 NEXTI:Y=Y+9
826 SOUND 7,&B111110:IF X>P+180 THEN PLAY "t120s11m9000o7l3c"
828 IF PLAY(1) THEN 828
830 X=P+36
832 NEXTJ
834 FORK=1TO400:NEXTK
850 DRAW "s4a0c4bm290,190"
854 DRAW "m-2,+1m+26,-26m-12,+15m-6,+10  m-2,+6 f1e1 m+4,-8m+4,-5m+8,-8m-6,+9d2m+8,-6m-4,+4m+2,+1m+8,-7bu3d1bd2m-1,+4m+7,-4d1m+6,-2r1e1 bm+4,-2d1m+20,-5m-84,+32
856 FORK=1TO400:NEXTK
858 FORI=P+450 TO P+400 STEP -2:COPY(I-200,44)-(I-1,54) TO (I-202,44):COPY(I,30)-(I+2,211) TO (I-2,30):PUTSPRITE4,(I/2-5,18),6,2:NEXT I
890 CC=6:L=3:P=220:Q=25:B=200:H=55:GOSUB960
900 C1=2:C2=3:COLOR=(C1,6,6,1):COLOR=(C2,7,1,1)
901 SETPAGE0,0:D=1:XS=8:C=C2:Y=50+6*9:A$="Tratamento de arquivos":L=LEN(A$)*8:X=246-L/2:LINE (X-20,Y-10)-STEP(L+40,26),C1,BF,PSET:LINE-(X-20,Y-10),C2,B:GOSUB5751:Y=Y-9
902 FORI=8TO13:COLOR=(I,I-8,0,0):NEXTI
903 SETPAGE0,0:GOSUB9150:COLOR1,0,0:J=15:LINE(0,211-J)-(511,211),0,BF:FORI=0TO6:Y=I*J:LINE(0,Y)-STEP(511,J-2),I+8,BF:LINE(0,Y+J-1)-STEP(511,0),0,,PSET:LINE(0,210-J-Y)-STEP(511,J-2),I+8,BF:LINE(0,209-J-Y)-STEP(511,0),0,,PSET
904 IF I=3 THEN COLOR=(14,7,0,0):COLOR=(15,7,7,7)
905 NEXTI:F=0:CT=15:CL=5:CC=6:CH=3:CK=4:CW=15:C2=7
907 RESTORE907:K=7:GOSUB9100:DATA0,0,333,334,222,247,223
909 COLOR=(15,6,6,6)
911 L=3:P=220:Q=25:B=200:H=55
913 LINE (P-10,Q-10)-STEP(B+30,H*L+30),CK,BF,PSET:LINE (P-10,Q-10)-STEP(B+30,H*L+30),1,B
915 LINE (P-10,Q-10)-STEP(60,-15),1:LINE-STEP(B+30,0),1:DRAW"c1nm-60,+15":LINE-STEP(0,H*L+10-10):LINE-STEP(-60,35+10)
917 PAINT (P,Q-11),C2,1:PAINT(P+B+21,Q),C2,1
919 A$="Produtos":GOSUB 950
921 A$="Revendedor":Q=Q+H+8:GOSUB 950
923 A$="Cliente":Q=Q+H+8:GOSUB 950:Q=Q-H-8
925 SETPAGESP,1:COLOR 1,0:Q=Q+3:A$="Revendedor":F=1:GOSUB950:SETPAGE0,0:Q=Q-3
927 P2=10:Q2=10
929 FORI=1TO25STEP2
931 COPY (P,Q)-STEP(B+5,H+3+I/3),1 TO (P-2-I*2+3,Q+I-3),0,TPSET
933 LINE (P-2-I*2+3,Q+I+H+I/3)-STEP(B,0),1
935 NEXT I
939 FORI=1TOH-3
941 COPY (P2,Q2)-STEP(B,I),1 TO (P-27,Q-I+15),0,TPSET
943 NEXTI
945 FORK=1TO500:NEXTK:GOTO 1000
950 'la
951 LINE (P,Q)-STEP(B,H+15*F),CL,BF
952 LINE (P,Q)-STEP(B,H+15*F),1,B
953 IF F>0 THEN LINE (P+9,Q)-STEP(0,-3):LINE-STEP(B-11,0):LINE-STEP(0,3):PAINT(P+15,Q-1),CC,1
954 LINE (P+40,Q+10)-STEP(B-80,12),CW,BF:LINE (P+40,Q+10)-STEP(B-80,12),1,B
955 C=1:X=P+B/2-LEN(A$)*8/2:Y=Q+12:D=1:GOSUB5700
956 LINE (P+30,Q+30)-STEP(B-60,12),CH,BF:LINE (P+30,Q+30)-STEP(B-60,12),1,B
957 RETURN
960 'card
961 SETPAGESP,1:COLOR 1,0:CLS
962 P2=10:Q2=10:PP=P:QQ=Q:P=P2:Q=Q2
963 DRAW "a0s4bm=p;,=q;bd9"
964 DRAW "c1u7e2r96f2d7r90f2d=h;l192u=h;u2"
965 PAINT (P+10,Q+10),CC,1
966 COLOR1,0:RESTORE 967
967 DATA    MP      ,  INFORMATICA,AL NHAMBIQUARAS 2095,MOEMA,CEP 04090
968 READA$:PSET(P+10,Q+2),0,TPSET:PRINT#1,A$:PSET(P+11,Q+2),0,TPSET:PRINT#1,A$
969 FORJ=1TO5:READA$:LINE(P+10,Q+J*9+5)-STEP(20,6),1,B
970 C=1:X=P+35:Y=Q+J*9+5:GOSUB5750:NEXTJ
971 SET PAGE 0,0
972 P=PP:Q=QQ
973 RETURN
1000 FORK=1TO500:NEXTK
1002 CA=11:COLOR=(CA,0,1,0):COLOR 15,CA,CA:CLS
1005 RESTORE 1005:K=9:GOSUB9100:DATA0,700,70,27,723,111,1,333,777
1010 CL=6:CB=7:CK=8:CG=9
1015 C=3:X=10:Y=0:A$="Aplicacoes Comerciais...":GOSUB5775
1020 M=10:C=2:P=60:Q=123:S=5:GOSUB1050
1025 M=20:C=3:P=240:Q=140:S=7:GOSUB1050
1030 M=10:C=4:P=15:Q=200:S=8:XO=P+4:YO=Q:GOSUB1050
1035 RESTORE1084:M=5:C=5:P=190:Q=170:S=5:GOSUB1050
1040 GOTO 1200
1050 LINE (P-14,Q-101)-STEP(258,110),CB,BF:LINE -(P-14,Q-101),CK,B:LINE (P-13,Q-101)-STEP(256,110),CK,B
1055 FORI=P-10 TO P+240 STEP M:LINE (I,Q-99)-STEP(0,106),CL:NEXT
1060 FORI=Q-99 TO Q+7   STEP M:LINE (P-10,I)-STEP(250,0),CL:NEXT
1065 LINE (P-10,Q-99)-STEP(250,106),CL,B
1070 LINE (P,Q)-STEP(0,-90),CG:LINE (P,Q)-STEP(222,0),CG
1075 FORI=2 TO 222-S STEP S
1077 IFC=2THENLINE (P+I,Q-1)-STEP(S-2,-RND(1)*50-I/5),C,BF
1079 IFC=3THEN X=P+I+2:Y=Q-RND(1)*50-I/5:CIRCLE(X,Y),3,C:PAINT(X,Y),C,C
1081 IFC=4THEN X=P+I+2:Y=Q-RND(1)*50-I/5:FORJ=0TO3STEP1:LINE(XO,YO-J)-(X,Y-J),C:NEXTJ:XO=X:YO=Y
1083 IFC=5THENREADJ:LINE (P+I,Q-J)-STEP(2,2),C,BF
1084 DATA52,60,66,72,76,79,79,78,75,71,65,58,51,43,36,30,25,21,20,20,22,25,31,37,44,52,59,66,71,76,79,79,79,76,72,66,59,52,44,37,31,26,22,20
1085 NEXT
1087 RETURN
1200 '
1400 C=4:SETPAGESP,1:CLS:S=2:X=16:Y=3:GOSUB 8800:SETPAGE0,0
1401 BC=0:COLOR15,BC,BC:SCREEN7,3
1402 COLOR=(BC,0,3,5):COLOR=(4,0,4,7):COLOR=(5,0,3,7):COLOR=(6,1,4,1)
1408 C=4:X=240:Y=20:S=1:GOSUB8800
1412 C=4:X=240:Y=20:S=1:GOSUB8900
1416 GOSUB 1600
1420 C=4:SETPAGESP,1:S=2:X=16:Y=3:GOSUB 8900:SETPAGE0,0
1424 LINE (130,63)-STEP(188,96),1,BF
1432 D=1:X=136:Y=68:A$="Frota M�vel:":C=15:GOSUB5700
1436 X=45:Y=98:E=+3:F=-1:FORI=1TO110
1440 COPY (X,Y)-STEP(180,75),1 TO (134,82),0,PSET
1444 X=X+E:Y=Y+F:IFX<45ORX>200THENE=-E
1448 IF Y<12 OR Y>100 THEN F=-F
1452 NEXTI
1456 FORK=1TO500:NEXTK
1458 COLOR15,1:CLS:COLOR=(0,0,0,0):FORI=2TO10:COLOR=(I,4+IMOD3,I/2,0):NEXTI
1460 SETPAGE1,1:COPY(0,0)-(511,211),1 TO (0,0),0:SETPAGE0,0:'FORI=0TO511STEP4:COPY(I,0)-STEP(3,211),1 TO (I,0),0:NEXTI
1464 F=5:ON INTERVAL=15 GOSUB 1500:INTERVAL ON
1468 C(0)=8:C(1)=9:C(2)=10:CO=1:R=7:R2=R/1.5
1472 RESTORE1472:DATAFilial Normal,Filial Especial,Organiza��o Local
1476 X=270:Y=151:CV=15:CT=1
1480 C=1:D=1:X=X+20:Y=Y+10:A$="Extremo Oriente":GOSUB5700:Y=Y+4
1484 FORI=0TO2:LINE(X,Y)-STEP(R,R2),C(I),BF:LINE-STEP(-R,-R2),1,B:X=X+24:READA$:GOSUB5700:X=X-24:NEXTI
1488 FORI=1TO45:RX=RND(1)*400+10:RY=RND(1)*160+6 : P=POINT(RX,RY):IF P=2ORP=3ORP=12 THEN LINE(RX,RY)-STEP(R,R2),C(RND(1)*1.5+IMOD2),BF:LINE-STEP(-R,-R2),CO,B
1492 NEXTI
1496 GOTO 1800
1500 COLOR=(C(0),F,0,0)
1504 COLOR=(C(1),F,F,0)
1508 COLOR=(C(2),0,F,F)
1512 F=7-F
1516 RETURN
1600 '
1602 R=35:VH=1/5
1604 PI=3.1415927536#
1606 CT=14:CW=15:CD=5
1608 FORI=7TO14:COLOR=(I,I-7,0,0):NEXTI
1610 COLOR=(CW,7,7,7)
1612 COLOR=(CD,7,4,0)
1613 P=40:Q=20:C1=390:C2=100
1615 SPRITE$(0)=STRING$(32,255):PUTSPRITE0,(C1/2-20,C2-15),4,0:PUTSPRITE1,(C1/2-4,C2-15),4,0:LINE(C1-40,C2-12)-STEP(80,25),0,BF
1616 SETPAGE0,0:CIRCLE (C1,C2),R,2:PAINT (C1,C2),CT,2:CIRCLE(C1,C2),R,1:FORI=8TO15:CIRCLE(C1,C2),R,I-1:NEXTI
1618 COPY (C1-R,C2-10)-(C1+R,C2+R*VH),0 TO (C1-R,C2-11),0,TPSET:COPY (C1-R,C2-9)-(C1+R,C2+R*VH),0 TO (C1-R,C2-11),0,TPSET
1620 SETPAGE0,0
1622 '
1624 RESTORE 1672
1626 C=15
1628 LINE (P-15,Q)-STEP(164,170),CW,BF
1630 LINE (P-15,Q)-STEP(164,170),1,B
1632 RESTORE 1672
1634 FORI=1TO3
1636 READ YH,YL,X:X=X+P:YH=YH+Q-20:YL=YL+Q-40
1638 FORY=YH TO YL STEP-4
1640 COPY (C1-R,C2-R*VH-4)-STEP(R*2,VH*2*R+4),0 TO (X,Y),0,TPSET
1642 NEXT Y
1644 LINE (X+R-17,Y+R*VH-4-3)-STEP(8*4-1,9),5,BF
1646 D=1:C=1:X=X+R-16:Y=Y+R*VH-6:A$=MID$(STR$(1987-I),2):GOSUB5700
1648 NEXT I
1650 RESTORE1656:C=1:FORK=1TO6:D=1:X=P+197:Y=123+9*K:READA$:GOSUB 5700:NEXTK
1651 LINE(C1-40,C2-12)-STEP(80,25),4,BF:PUTSPRITE0,(0,209),0,0:PUTSPRITE1,(0,209),0,0
1652 RETURN
1654 'data
1656 DATA"Extremo Oriente, 1984 - 1986"
1658 DATA"----------------------------"
1660 DATA"Indonisia  -Borneo      16 %"
1662 DATA"Tailandia  -Bangkok     69 %"
1664 DATA"Birmania   -Mandalay     8 %"
1666 DATA"Laos       -Vietna       7 %"
1672 DATA140, 60,20
1674 DATA150,100,60
1676 DATA160,140,0
1800 '
2000 '
2100 'GOTO 40
3974 RETURN
3999 SETPAGE1,1:COLOR=(0,0,1,5):COPY(0,0)-(511,211),1 TO (0,0),0:SETPAGE0,0:RETURN
5664 COLORC,0:PRESET(X  ,Y),0,TPSET:PRINT#1,A$
5700 '
5702 IF D>0 THENPRESET(X+1,Y),0,TPSET:PRINT#1,A$:D=0
5704 Y=Y+9:RETURN
5720 X=256-LEN(A$)*XS/2:GOTO 5700
5750 XS=6+D
5751 XB=X:COLOR C,0
5752 FORI=1TOLEN(A$)
5753 IF MID$(A$,I,1)="\" THEN X=XB-XS*I:Y=Y+9:GOTO5756
5754 IF MID$(A$,I,1)="*" THEN X=X-XS:T=C:C=C2:C2=T:COLOR C,0:GOTO5756
5755 PSET (X+XS*I-XS,Y),0,TPSET:PRINT#1,MID$(A$,I,1)
5756 IF D>0 THEN PSET (X+XS*I-XS-1,Y),0,TPSET:PRINT#1,MID$(A$,I,1)
5757 NEXTI
5758 Y=Y+9:D=0:RETURN
5775 SETPAGE0,1:COLOR C,0:PSET(X,Y),0,PSET
5777 PRINT#1,A$
5779 IF D>0 THEN PSET(X+1,Y),0,PSET:PRINT#1,A$:D=0
5781 FORI=14TO0STEP-1:COPY(X,Y+I/2)-STEP(8*LEN(A$)-1,0),1 TO (X,Y+I),0,TPSET:NEXTI
5783 Y=Y+18:SETPAGE0,0
5785 RETURN
8000 PSET (0,0),0,PSET
8010 DRAW"S=S;C=C;BM=X;,=Y;"
8020 DRAW"M+7,-26R6M+3,+12M+3,-12R6M+5,+20R13E1U2H1L7M-3,-1H2M-1,-3U5M+1,-3E2M+3,-1R22M+4,+7M+4,-7R8M-8,+14M+8,+14L8M-4,-7M-4,+7L8M+8,-14M-4,-7L17G1D2F1R7"
8030 DRAW"M+3,+1F2M+1,+3D5M-1,+3G2M-3,1L19M-3,-12M-3,+12L6M-3,-12M-3,+12L7U1"
8040 PAINT (X+2,Y),C,C
8050 'LINE (X+240,Y+3)-STEP(21,-S*28/4),2,BF
8051 'LINE (X+265,Y+3)-STEP(21,-S*28/4),2,BF
8055 C=2:PI=3.14159265358#
8060 X1=X+80*S/4:Y1=Y+1*S/4:S=S/4
8061 LINE(X1,Y1)-STEP(S*20,-S*6),C,BF
8062 CIRCLE(X1+10*S,Y1-S*18),S*4,C
8063 LINE(X1+12*S,Y1-S*16)-(X1,Y1-S*6),C
8064 CIRCLE(X1+10*S,Y1-S*18),S*10,C
8065 LINE(X1+16*S,Y1-S*10)-(X1+S*11,Y1-S*6),C
8066 LINE(X1,Y1-S*18)-STEP(S*7+1,0),C
8067 PAINT(X1+S*10,Y1-S*23),C
8070 RETURN
8500 FOR K=1TO2:X=X+1
8505 COLORC:DRAW"c=c;s=s;a0bm=x;,=y;nd24l60nd24bm+4,+9":COLORC
8510 DRAW"u6r5m+1,+1d2m-2,+1l4bm+10,-1nu3nd3r6nu3d3br4u6br4d6r4 br4nu6  br4u6r4m+2,+1d2m-2,+1l4bm+10,+1   f1r4e1u1h1l4h1u1e1r4f1"
8515 CIRCLE(X-30*S/4,Y+6*S),30*S/4,C
8520 CIRCLE(X-30*S/4,Y+6*S),26*S/4,C
8525 DRAW"bm=x;,=y;bm-6,+22"
8530 A$="bm-48,+2r2m+4,-2r8m+4,+2r8m+4,-2r8m+4,+2r6"
8535 DRAW"xA$;xA$;xA$;"
8540 DRAW"bm-44,-10r12u5d2r4l10u1d7bm+34,+9l12d5u2l4r10d1u7"
8545 NEXTK
8550 RETURN
8600 FOR K=1TOS/2:X=X+1
8605 COLORC:DRAW"c=c;s=s;a0bm=x;,=y;"
8610 DRAW"u6r5m+1,+1d2m-2,+1l4bm+10,-1nu3nd3r6nu3d3br4u6br4d6r4 br4nu6  br4u6r4m+2,+1d2m-2,+1l4bm+10,+1   f1r4e1u1h1l4h1u1e1r4f1"
8615 NEXTK
8620 RETURN
8800 'landcard
8802 P=X*S:Q=Y*S
8804 LINE(P-4,Q-2)-STEP(8+220*S,4+100*S),C,BF
8806 LINE(P,Q)-STEP(220*S,100*S),1,B
8808 LINE(P-4,Q-2)-STEP(220*S+8,100*S+4),1,B:PAINT(P-1,Q-1),15,1
8810 COLOR1
8812 I=S*4:DRAW"S=i;BM=p;,=q;"
8816 DRAW"R2F1R3F3D1F3R1U1H1U1R2D1F7R2D2F1D3G1F2R1ND1R2ND1R2ND1E2U6R1F1D1F1R2F1D3F1G1F1D1":DRAW"F4D1G1D1G1D2R3F2D2F1D1F1D1L2H1D2F3D1F1D10"
8820 DRAW"F1R1U3E1F1D2F4R1D1F2D1R2F4D1F1D1F2U1F5D2F2L4":DRAW"F2R2F3D2R4F3R1F2R1F1R7E1F1R1E1"
8824 DRAW"H3D1L1H1U10H1U1H1U2H2L1U3L1H3U1H1L3H1L1U1H1":DRAW"L1H3G2H2U2H1L1H1U1H3L1G1D2L1H1U1H1U4":DRAW"E1H1E1H1E1H1U1E1U1H1U2E1R11"
8828 DRAW"F1G1D1G1D2G1R2E1R1F2R4U2R1F1R3E1F1R1D1G1D1R3":DRAW"E1R2E1R1F2D1F1D1G1D2G2R2D3F1R1E1U1R1E1U1E1U2":DRAW"R1F1E1R3E1R3E1R1U2R2D1R2U1E3F2U2E2U3F2U3H1"
8830 DRAW"U3E1U2L2H1U2H1U4L1H3U1E1U3E2U2E1R1E1U1R3E1U1":DRAW"R3F1R4E1R1F2R1F1D1F1R4D2L1D1G1F1R2":DRAW"E1R1E2U3E1F1R3E2R2D2F1E1R1E1R3E2":DRAW"R2E1R3E1R4U3E4D6R1F1E1F1R2E1U1E4"
8834 P=S*(X+52):Q=S*(Y+100):DRAW"s=i;BM=p;,=q;"
8836 DRAW"H1L1H1E1R2U1H1L7H2L1H2U1E1H1L1U1H1U1H2L1U1H1U2":DRAW"H1L1U2H2L1H1U3R2F1D1R1F2R2F1R1E1R5F1R1D2G1F2L1":DRAW"F2R2F1R3F1R1F1R4F1D2F1R1F1R2F2R2F1R1F2D1R2":DRAW"U2H1R2F2R2D4R1E1R1F2"
8840 P=S*(X+142):Q=S*(Y+19):DRAW"s=i;BM=p;,=q;"
8842 DRAW"R2E1R2F1R2E1F1R2E2R1F1D2F1G2L2D2G1L2H1G2L1H1L2H1L1H1U1H1U2E1"
8846 P=S*(X+49):Q=S*(Y+54):DRAW"bm=p;,=q;"
8848 DRAW"R1E9H8E1U2H2E1U1L2H3E1R2E3R2E1U1E1R1":DRAW"E3R3E5R2F2R2E1R1F2R3E1R1E1R2F3D1G1D1F1":DRAW"R8F1R2E1R3F3D1G1D2F2G1D2G1L5G1L1H3L2H1L2H1L2":DRAW"G2L2D2F1D2G1D1F1D2G2"
8852 P=S*(X+66):Q=S*(Y+71):DRAW"BM=p;,=q;"
8854 DRAW"R1E1R1F1R2E1R1F3R3E1R2F1R2E1R2"
8858 P=S*(X+93):Q=S*(Y+48):DRAW"BM=p;,=q;"
8860 DRAW"U1E2U1R1E1R1F2R5E2R2E1R3E1R1E1R1E2U3L3H1L1H2L1G1L3":DRAW"BM+12,+2E1U2H1U2H1U4H2U2E1U3H1L2G1L1H1U3H1U2H2L2H3U1":DRAW"R7F1R3E1R3U2E1F1R2F1E1R1F3R2F3"
8864 P=S*(X+78):Q=S*Y:DRAW"BM=p;,=q;"
8866 DRAW"D4F2R2F1R5F2D1F1D1G3BM+3,-3E1F2R2U3H1E2R2F2E3"
8868 RETURN
8900 '
8905 PAINT(S*(X+40),S*(Y+5)),2,1
8910 PAINT(S*(X+70),S*(Y+30)),6,1
8915 PAINT(S*(X+60),S*(Y+95)),2,1
8920 PAINT(S*(X+90),S*(Y+80)),3,1
8925 PAINT(S*(X+100),S*(Y+40)),3,1
8930 PAINT(S*(X+100),S*(Y+15)),2,1
8935 PAINT(S*(X+90),S*(Y+5)),3,1
8940 PAINT(S*(X+150),S*(Y+5)),3,1
8945 PAINT(S*(X+195),S*(Y+5)),3,1
8950 PAINT(S*(X+150),S*(Y+25)),3,1
8955 PAINT(S*(X+120),S*(Y+10)),6,1
8960 RETURN
9100 FORI=1TOK:READJ:COLOR=(I,J/100,(JMOD100)/10,JMOD10):NEXTI:RETURN
9150 FORI=0TO31:PUTSPRITEI,(0,209),0,I:NEXTI:RETURN