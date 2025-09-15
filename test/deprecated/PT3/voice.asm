; VOICE
; https://marmsx.msxall.com/projetos/dsg/players.php

MAIN:
  DI 		        ; Desabilita interrupção
    IN   A,(&HA8)	; Obtém configuração dos slots
    LD   E,A	    ; Salva no reg. E
    LD   B,4	    ; 
ROT:	
    SRL  A		    ; Desloca o valor de A 4 vezes
    DJNZ ROT	    ;
    ADD  A,E	    ; Soma A com E
    OUT  (&HA8),A	; Configura todas as páginas como RAM
    LD   HL,DATA	; Endereço inicial do PCM
LOOP:	
    LD   C,(HL)	    ; Lê próximo octeto (8 amostras)
    LD   B,8	    ; Repete 8x
LPI:	
    RL   C		    ; Desloca 1 bit para a esquerda
    IN   A,(&HAA)	; Lê o estado da porta C da PPI
    JR   C,TONE	    ; Verifica de CY=1
      AND  &H7F	    ; Se não, click=0
      JR   CHTONE	; Salta para CHtone
TONE:	
    OR   &H80	    ; Se sim, click=1
CHTONE:	
    OUT  (&HAA),A	; Envia resultado para PPI
    LD   E,14	    ; Define um delay de 14
DELAY:	
    DEC  E		    ;
    JR   NZ,DELAY	; Delay
    DJNZ LPI	    ;
    INC  HL		    ; Passa para o próximo octeto
    LD   A,H	    ;
    CP   &H40	    ; Verifica de HL=&H4000
    JR   NZ,LOOP	; Se não for, repete LOOP
    IN   A,(&HA8)	; Obtém configuração dos SLOTS
    AND  &HF0	    ; Seta páginas 0 e 1 como ROM
    OUT  (&HA8),A	; Troca
  EI 		        ; Habilita interrupções
  RET		        ; Retorna ao Basic

DATA:

