/***
 * @file z80.h
 * @brief Z80 interface class header (bridge design pattern),
 *        specialized as a Z80 code stream writer for MSX system
 * @author Amaury Carvalho (2025)
 * @note
 *   https://refactoring.guru/design-patterns/bridge
 *   Z80 Opcodes:
 *     http://z80-heaven.wikidot.com/instructions-set
 *     https://clrhome.org/table/
 */

#ifndef Z80_H
#define Z80_H

#include <string.h>

#include <queue>

/***
 * @class IZ80
 * @brief Z80 interface class (bridge design pattern),
 * specialized as a Z80 code stream writer for MSX system
 */
class IZ80 {
 protected:
  unsigned char* code;  //[20*0xFFFF];

  int code_pointer;
  unsigned char* last_code[5];

  void addByte(unsigned char byte);
  void addWord(unsigned int word);
  void addWord(unsigned char byte1, unsigned char byte2);
  void addCmd(unsigned char byte, unsigned int word);
  void addCodeByte(unsigned char byte);
  void pushLastCode();
  void popLastCode();

  /***
   * @brief NOP does nothing for 4 clock cycles.
   * See [nop](http://z80-heaven.wikidot.com/instructions-set:nop) for more
   * information.
   * @result 1 byte, 4 t-states
   */
  void addNop();
  void addExAF();
  void addExDEHL();
  void addExiSPHL();
  void addExx();
  void addEI();
  void addDI();
  void addPushAF();
  void addPushHL();
  void addPushBC();
  void addPushDE();
  void addPopAF();
  void addPopHL();
  void addPopBC();
  void addPopDE();
  void addPopIX();
  void addPopIY();
  void addXorA();
  void addXorH();
  void addXorE();
  void addXorD();
  void addAndA();
  void addAndD();
  void addAnd(unsigned char n);
  void addOr(unsigned char n);
  void addOrH();
  void addOrL();
  void addOrB();
  void addOrC();
  void addOrE();
  void addCpL();
  void addCpB();
  void addCp(unsigned char n);
  void addIncA();
  void addIncH();
  void addIncL();
  void addIncD();
  void addIncHL();
  void addIncDE();
  void addDecHL();
  void addAdd(unsigned char n);
  void addAddH();
  void addAddL();
  void addAddA();
  void addAddHLHL();
  void addAddHLBC();
  void addAddHLDE();
  void addAdcA(unsigned char n);
  void addSub(unsigned char n);
  void addSbcHLDE();
  void addSbcHLBC();
  void addRRCA();
  void addRLA();
  void addSRAH();
  void addSRLH();
  void addRRL();
  void addRRA();
  void addCPL();
  void addLDI();
  void addLDIR();
  void addDecA();
  void addDecE();
  void addRet();
  void addRetC();
  void addLdA(unsigned char n);
  void addLdAB();
  void addLdAC();
  void addLdAH();
  void addLdAL();
  void addLdAE();
  void addLdAD();
  void addLdAiHL();
  void addLdAiDE();
  void addLdHA();
  void addLdLA();
  void addLdLC();
  void addLdLH();
  void addLdL(unsigned char n);
  void addLdHE();
  void addLdHC();
  void addLdHL();
  void addLdH(unsigned char n);
  void addLdLiHL();
  void addLdBA();
  void addLdBH();
  void addLdBL();
  void addLdBE();
  void addLdBiHL();
  void addLdB(unsigned char n);
  void addLdC(unsigned char n);
  void addLdEiHL();
  void addLdEA();
  void addLdEL();
  void addLdCA();
  void addLdCB();
  void addLdCE();
  void addLdCD();
  void addLdCH();
  void addLdCL();
  void addLdCiHL();
  void addLdDA();
  void addLdDL();
  void addLdDH();
  void addLdDiHL();
  void addLdHB();
  void addLdHL(int n);
  void addLdHLmegarom();
  void addLdBC(int n);
  void addLdDE();
  void addLdDE(int n);
  void addLdIX(int n);
  void addLdAii(int i);
  void addLdHLii(int i);
  void addLdBCii(int i);
  void addLdDEii(int i);
  void addLdIXii(int i);
  void addLdIYii(int i);
  void addLdiiA(int i);
  void addLdiiHL(int i);
  void addLdiiDE(int i);
  void addLdiiSP(int i);
  void addLdSPii(int i);
  void addLdSPHL();
  void addLdiHL(unsigned char n);
  void addLdiHLA();
  void addLdiHLB();
  void addLdiHLC();
  void addLdiHLD();
  void addLdiHLE();
  void addLdiDEA();

  /***
   * @brief Relative jumps to the address.
   * This means that it can only jump between 128 bytes ahead or behind.
   * JR takes up one less byte than JP, but is also slower.
   * Weigh the needs of the code at the time before choosing one over the other
   * (speed vs. size). See [jr
   * n](http://z80-heaven.wikidot.com/instructions-set:jr) for more information.
   * @param n The signed value n is added to PC.
   * The jump is measured from the start of the instruction opcode.
   * @result 2 bytes, 12 t-states
   */
  void addJr(unsigned char n);

  /***
   * @brief Relative conditional jumps to the address.
   * See [jr Z,n](http://z80-heaven.wikidot.com/instructions-set:jr) for more
   * information.
   * @param n The signed value n is added to PC.
   * The jump is measured from the start of the instruction opcode.
   * @result 2 bytes, 12 (cond met) or 7 (not met) t-states
   */
  void addJrZ(unsigned char n);

  /***
   * @brief Relative conditional jumps to the address.
   * See [jr NZ,n](http://z80-heaven.wikidot.com/instructions-set:jr) for more
   * information.
   * @param n The signed value n is added to PC.
   * The jump is measured from the start of the instruction opcode.
   * @result 2 bytes, 12 (cond met) or 7 (not met) t-states
   */
  void addJrNZ(unsigned char n);

  /***
   * @brief Relative conditional jumps to the address.
   * See [jr C,n](http://z80-heaven.wikidot.com/instructions-set:jr) for more
   * information.
   * @param n The signed value n is added to PC.
   * The jump is measured from the start of the instruction opcode.
   * @result 2 bytes, 12 (cond met) or 7 (not met) t-states
   */
  void addJrC(unsigned char n);

  /***
   * @brief Relative conditional jumps to the address.
   * See [jr NC,n](http://z80-heaven.wikidot.com/instructions-set:jr) for more
   * information.
   * @param n The signed value n is added to PC.
   * The jump is measured from the start of the instruction opcode.
   * @result 2 bytes, 12 (cond met) or 7 (not met) t-states
   */
  void addJrNC(unsigned char n);
  void addJp(int n);
  void addJpZ(int n);
  void addJpNZ(int n);
  void addCall(unsigned int word);

 public:
  IZ80();

  int code_size;
};

#endif  // Z80_H
