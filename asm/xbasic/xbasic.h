#ifndef XBASIC_H_INCLUDED
#define XBASIC_H_INCLUDED

#include <string>
#include <vector>
#include <stack>

class XBasic {
	
    private:
       int hl, bc, de, af, ix, iy, sp;
	   unsigned char *h, *l, *b, *c, *d, *e, *a, *f;
       	   
	   stack<int> stk;
	   
	   int 	RAMAD1,
			FORCLR,
			BAKCLR,
			BDRCLR,
			ATRBYT,
	        CURLIN,
            BUF,
            FNPTR, 
			DUMMY,
			DY,
			NX,
			NY,
			ARGT,
			VALTYP,
			TXTTAB,
			DATLIN,
			SUBFLG,
			SAVSTK,
			STREND,
			DATPTR,
			DEFTBL,
			SWPTMP,
			SWPTMP1,
			STRBUF,
			DAC,
			MODE,
			ONGSBF,
			TRPTBL,
			TRPTBL_STOP,
			TRPTBL_SPRITE,
			TRPTBL_STRIG,
			TRPTBL_INTERVAL,
			INTFLG,
			JIFFY,
			SCRMOD,
			GRPACX,
			GRPACY,
			EXPTBL,
			PROCNM,
			VARTMP1, VARTMP2, VARTMP3, VARTMP4;
	   
	   void setword(int addr, int value);
	   int getword(int addr);
	   
	   void call_run_parse();

       void proc_l41f1();
       void proc_l420f();
    
       int sub_4d0a();
       int sub_42d9();
	   int sub_4387();
       int sub_4258();       
	   int sub_648a();
        
	public:
       XBasic();
	   
	   unsigned char ram[0xFFFF];
	   
	   void call_run(int address);
	   
}

	
#endif // XBASIC_H_INCLUDED
