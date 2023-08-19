' O TESOURO PERDIDO
' Original de Renato Degiovani
' Adaptado por Amaury Carvalho, 2023
' Para compilar, digite no terminal: 
'   msxbas2rom -c -x tesperd.bas

FILE "tesperd.akm"                  ' 0
FILE "tesperd.akx"                  ' 1
FILE "SCREEN1.SC2"                  ' 2
FILE "SCREEN2.SC2"                  ' 3
FILE "SCREEN3.SC2"                  ' 4
FILE "SCREEN4.SC2"                  ' 5
FILE "SCREEN5.SC2"                  ' 6
FILE "SCREEN6.SC2"                  ' 7
FILE "SCREEN7.SC2"                  ' 8
FILE "SCREEN8.SC2"                  ' 9

05 CMD PLYLOAD 0, 1                 ' Carrega musica e efeitos do Arkos Tracker

10 SCREEN 2,,0 : COLOR 1, 15
11 SCREEN LOAD 2                    ' Carrega tela de abertura
12 CMD PLYSONG 0 : CMD PLYPLAY      ' Toca tema musical de abertura
13 LOCATE 60, 130 : PRINT "O TESOURO PERDIDO"
14 LOCATE 64, 150 : PRINT "Renato Degiovani"

20 Frase$ = ""
21 Lamp = 0
22 Oleo = 0
23 Vaso = 0
24 Prta = 0
29 GOSUB 912
30 CMD PLYSONG 1 : CMD PLYPLAY      ' Toca tema musical do jogo 

100 SCREEN LOAD 3                   ' Carrega fase inicial do jogo
110 LOCATE 20, 110 : PRINT "Voce esta ao lado de um"
111 LOCATE 20, 120 : PRINT "buraco no chao."
112 IF Lamp=1 GOTO 120
113 LOCATE 20, 130 : PRINT "Aqui tem um lampiao."
120 LOCATE 20, 150 : PRINT "E = entre no buraco"
123 LOCATE 20, 160 : PRINT "T = seus objetos"
124 IF Lamp=1 GOTO 130
128 LOCATE 20, 170 : PRINT "P = pegue o lampiao"
130 GOSUB 940
131 IF Frase$="P" or Frase$ = "p" GOTO 150
132 IF Frase$="E" or Frase$ = "e" GOTO 200
133 IF Frase$="T" or Frase$ = "t" GOTO 190
140 GOSUB 900
145 GOTO 100

150 Lamp=1
155 GOSUB 990
160 LOCATE 20, 110 : PRINT "Peguei o lampiao."
165 GOSUB 912
167 GOTO 100

190 GOSUB 800
192 GOTO 100

200 SCREEN LOAD 4 
201 IF Lamp=1 GOTO 210
202 LOCATE 20, 110 : PRINT "Aqui esta escuro."
203 GOSUB 912
204 GOTO 100
210 LOCATE 20, 110 : PRINT "Voce esta dentro do"
212 LOCATE 20, 120 : PRINT "buraco, numa caverna."
225 LOCATE 20, 130 : PRINT "T = seus objetos"
226 LOCATE 20, 140 : PRINT "X = sair da caverna"
227 LOCATE 20, 150 : PRINT "S = ir para o sul"
228 LOCATE 20, 160 : PRINT "L = ir para leste"
229 LOCATE 20, 170 : PRINT "O = ir para oeste"
230 GOSUB 940           
231 IF Frase$="X" OR Frase$="x" GOTO 100
232 IF Frase$="T" or Frase$="t" GOTO 290
233 IF Frase$="L" or frase$="l" GOTO 300
234 IF Frase$="S" or frase$="s" GOTO 400
235 IF Frase$="O" or frase$="o" GOTO 500
240 GOSUB 900
245 GOTO 200
290 GOSUB 800
292 GOTO 200

300 SCREEN LOAD 6+Oleo
310 LOCATE 20, 110 : PRINT "Voce esta numa sala da"
311 LOCATE 20, 120 : PRINT "caverna."
313 IF Oleo=1 GOTO 325
314 LOCATE 20, 130 : PRINT "Tem uma poca de oleo aqui."
316 IF Vaso=0 GOTO 325
317 LOCATE 20, 150 : PRINT "P = ponha o oleo no vaso"
325 LOCATE 20, 160 : PRINT "T = seus objetos"
326 LOCATE 20, 170 : PRINT "O = ir para oeste"
330 GOSUB 940
331 IF Frase$="T" OR Frase$="t" GOTO 390
332 IF Frase$="O" or frase$="o" GOTO 200
333 IF Frase$="P" or frase$="p" GOTO 380
340 GOSUB 900
345 GOTO 300

380 Oleo=1
381 GOSUB 990
382 LOCATE 20, 110 : PRINT "Ok, coloquei!"
384 GOSUB 912
386 GOTO 300
390 GOSUB 800
392 GOTO 300

400 SCREEN LOAD 5
410 LOCATE 20,110 : PRINT "Voce esta num corredor"
411 LOCATE 20,120 : PRINT "estreito, onde tem uma porta"
414 IF Prta=1 GOTO 428
415 LOCATE 20,130 : PRINT "A Porta esta travada e a"
416 LOCATE 20,140 : PRINT "dobradica esta enferrujada."
418 IF Vaso=0 GOTO 429
419 IF Oleo=0 GOTO 429
420 LOCATE 20,150 : PRINT "P = passe oleo na dobradica"
425 IF Prta=0 GOTO 429
428 LOCATE 20,160 : PRINT "S = ir para o sul"
429 LOCATE 20,170 : PRINT "N = ir para o norte"
430 GOSUB 940
431 IF Frase$="T" or Frase$="t" GOTO 490
432 IF Frase$="N" or Frase$="n" GOTO 200
433 IF Frase$="S" or Frase$="s" GOTO 600
434 IF Frase$="P" or Frase$="p" GOTO 480
440 GOSUB 900
445 GOTO 400
480 Prta=1
481 GOSUB 990
482 LOCATE 20, 110 : PRINT "Passei o oleo."
484 GOSUB 912
485 GOTO 400
490 GOSUB 800
495 GOTO 400

500 IF Vaso=0 THEN SCREEN LOAD 8 ELSE SCREEN LOAD 7
510 LOCATE 20,110 : PRINT "Voce esta numa sala da"
511 LOCATE 20,120 : PRINT "caverna."
513 IF Vaso=1 GOTO 525
514 LOCATE 20,130 : PRINT "Aqui tem um vaso"
517 LOCATE 20,150 : PRINT "P = pegue o vaso"
525 LOCATE 20,160 : PRINT "T = seus objetos"
526 LOCATE 20,170 : PRINT "L = ir para leste"
530 GOSUB 940
531 IF Frase$="T" OR Frase$="t" GOTO 590
532 IF Frase$="L" or Frase$="l" GOTO 200
533 IF Frase$="P" or Frase$="p" GOTO 580
540 GOSUB 900
545 GOTO 500
580 Vaso=1
581 GOSUB 990
582 LOCATE 20, 110 : PRINT "Peguei o vaso."
584 GOSUB 912
585 GOTO 500
590 GOSUB 800
595 GOTO 500

600 IF Ouro=0 THEN SCREEN LOAD 9 ELSE SCREEN LOAD 7
610 LOCATE 20, 110 : PRINT "Voce esta na sala do tesouro."
614 IF Ouro=1 GOTO 625
616 LOCATE 20, 120 : PRINT "Aqui tem uma barra de ouro."
620 LOCATE 20, 150 : PRINT "P = pegue a barra de ouro"
625 LOCATE 20, 160 : PRINT "T = seus objetos"
626 LOCATE 20, 170 : PRINT "N = ir para o norte"
630 GOSUB 940
631 IF Frase$="T" OR Frase$="t" GOTO 690
632 IF Frase$="N" or Frase$="n" GOTO 400
633 IF Frase$="P" or Frase$="p" GOTO 680
640 GOSUB 900
645 GOTO 600
680 Ouro=1
681 GOSUB 990
682 LOCATE 20, 110 : PRINT "Peguei o ouro."
684 GOSUB 912
685 GOTO 600
690 GOSUB 800
695 GOTO 600

800 GOSUB 990
810 LOCATE 20, 110 : PRINT "Voce tem os objetos:"
820 IF Lamp=1 THEN LOCATE 20, 130 : PRINT " - um lampiao"
830 IF Vaso=1 THEN LOCATE 20, 140 : PRINT " - um vaso"
840 IF Ouro=1 THEN LOCATE 20, 150 : PRINT " - uma barra de ouro"
899 GOTO 912

900 GOSUB 990
910 LOCATE 20, 110 : PRINT "Nao entendi o q eh p/fazer..."
911 CMD PLYSOUND 2                  ' Efeito de opção errada

912 LOCATE 54, 170 : PRINT "Pressione uma tecla"
920 Frase$ = INKEY$
930 IF Frase$="" GOTO 920
931 GOTO 950

940 Frase$ = INPUT$(1) 

950 CMD PLYSOUND 4                  ' Efeito de opção selecionada
951 RETURN

990 LINE (20, 110)-(250,180),15,BF : RETURN


