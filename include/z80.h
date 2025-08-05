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
   * @note [nop](http://z80-heaven.wikidot.com/instructions-set:nop)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addNop();

  /***
   * @brief EX AF,AF' exchanges the main and alternate accumulator and flags
   * registers.
   * @note [EX AF,AF'](http://z80-heaven.wikidot.com/instructions-set:ex-af-af)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addExAF();

  /***
   * @brief EX DE,HL exchanges DE and HL register contents.
   * @note [EX DE,HL](http://z80-heaven.wikidot.com/instructions-set:ex-de-hl)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addExDEHL();

  /***
   * @brief EX (SP),HL exchanges HL with the memory contents pointed to by SP.
   * @note [EX (SP),HL](http://z80-heaven.wikidot.com/instructions-set:ex-sp-hl)
   * @result opcode size: 1 byte; clock cycles: 19 t-states
   */
  void addExiSPHL();

  /***
   * @brief EXX exchanges BC, DE, HL with their shadow registers.
   * @note [EXX](http://z80-heaven.wikidot.com/instructions-set:exx)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addExx();

  /***
   * @brief EI enables interrupts after the next instruction.
   * @note [EI](http://z80-heaven.wikidot.com/instructions-set:ei)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addEI();

  /***
   * @brief DI disables maskable interrupts immediately.
   * @note [DI](http://z80-heaven.wikidot.com/instructions-set:di)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addDI();

  /***
   * @brief PUSH AF pushes the AF register pair onto the stack.
   * @note [PUSH AF](http://z80-heaven.wikidot.com/instructions-set:push)
   * @result opcode size: 1 byte; clock cycles: 11 t-states
   */
  void addPushAF();

  /***
   * @brief PUSH HL pushes the HL register pair onto the stack.
   * @note [PUSH HL](http://z80-heaven.wikidot.com/instructions-set:push)
   * @result opcode size: 1 byte; clock cycles: 11 t-states
   */
  void addPushHL();

  /***
   * @brief PUSH BC pushes the BC register pair onto the stack.
   * @note [PUSH BC](http://z80-heaven.wikidot.com/instructions-set:push)
   * @result opcode size: 1 byte; clock cycles: 11 t-states
   */
  void addPushBC();

  /***
   * @brief PUSH DE pushes the DE register pair onto the stack.
   * @note [PUSH DE](http://z80-heaven.wikidot.com/instructions-set:push)
   * @result opcode size: 1 byte; clock cycles: 11 t-states
   */
  void addPushDE();

  /***
   * @brief POP AF pops the AF register pair from the stack.
   * @note [POP AF](http://z80-heaven.wikidot.com/instructions-set:pop)
   * @result opcode size: 1 byte; clock cycles: 10 t-states
   */
  void addPopAF();

  /***
   * @brief POP HL pops the HL register pair from the stack.
   * @note [POP HL](http://z80-heaven.wikidot.com/instructions-set:pop)
   * @result opcode size: 1 byte; clock cycles: 10 t-states
   */
  void addPopHL();

  /***
   * @brief POP BC pops the BC register pair from the stack.
   * @note [POP BC](http://z80-heaven.wikidot.com/instructions-set:pop)
   * @result opcode size: 1 byte; clock cycles: 10 t-states
   */
  void addPopBC();

  /***
   * @brief POP DE pops the DE register pair from the stack.
   * @note [POP DE](http://z80-heaven.wikidot.com/instructions-set:pop)
   * @result opcode size: 1 byte; clock cycles: 10 t-states
   */
  void addPopDE();

  /***
   * @brief POP IX pops the IX register pair from the stack.
   * @note [POP IX](http://z80-heaven.wikidot.com/instructions-set:pop)
   * @result opcode size: 2 bytes; clock cycles: 14 t-states
   */
  void addPopIX();

  /***
   * @brief POP IY pops the IY register pair from the stack.
   * @note [POP IY](http://z80-heaven.wikidot.com/instructions-set:pop)
   * @result opcode size: 2 bytes; clock cycles: 14 t-states
   */
  void addPopIY();

  /***
   * @brief XOR A performs a bitwise XOR of A with A, setting A to 0.
   * @note [XOR A](http://z80-heaven.wikidot.com/instructions-set:xor)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addXorA();

  /***
   * @brief XOR H performs a bitwise exclusive OR between A and H.
   * @note [XOR](http://z80-heaven.wikidot.com/instructions-set:xor)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addXorH();

  /***
   * @brief XOR E performs a bitwise exclusive OR between A and E.
   * @note [XOR](http://z80-heaven.wikidot.com/instructions-set:xor)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addXorE();

  /***
   * @brief XOR D performs a bitwise exclusive OR between A and D.
   * @note [XOR](http://z80-heaven.wikidot.com/instructions-set:xor)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addXorD();

  /***
   * @brief AND A performs a bitwise AND between A and itself (effectively tests
   * if A is zero).
   * @note [AND](http://z80-heaven.wikidot.com/instructions-set:and)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addAndA();

  /***
   * @brief AND D performs a bitwise AND between A and D.
   * @note [AND](http://z80-heaven.wikidot.com/instructions-set:and)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addAndD();

  /***
   * @brief AND n performs a bitwise AND between A and an immediate value n.
   * @note [AND](http://z80-heaven.wikidot.com/instructions-set:and)
   * @param n Immediate 8-bit value to be ANDed with A.
   * @result opcode size: 2 bytes; clock cycles: 7 t-states
   */
  void addAnd(unsigned char n);

  /***
   * @brief OR n performs a bitwise OR between A and an immediate value n.
   * @note [OR](http://z80-heaven.wikidot.com/instructions-set:or)
   * @param n Immediate 8-bit value to be ORed with A.
   * @result opcode size: 2 bytes; clock cycles: 7 t-states
   */
  void addOr(unsigned char n);

  /***
   * @brief OR H performs a bitwise OR between A and H.
   * @note [OR](http://z80-heaven.wikidot.com/instructions-set:or)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addOrH();

  /***
   * @brief OR L performs a bitwise OR between A and L.
   * @note [OR](http://z80-heaven.wikidot.com/instructions-set:or)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addOrL();

  /***
   * @brief OR B performs a bitwise OR between A and B.
   * @note [OR](http://z80-heaven.wikidot.com/instructions-set:or)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addOrB();

  /***
   * @brief OR C performs a bitwise OR between A and C.
   * @note [OR](http://z80-heaven.wikidot.com/instructions-set:or)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addOrC();

  /***
   * @brief OR E performs a bitwise OR between A and E.
   * @note [OR](http://z80-heaven.wikidot.com/instructions-set:or)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addOrE();

  /***
   * @brief CP L compares register A with L by internally subtracting L from A.
   * @note [CP](http://z80-heaven.wikidot.com/instructions-set:cp)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addCpL();

  /***
   * @brief CP B compares register A with B by internally subtracting B from A.
   * @note [CP](http://z80-heaven.wikidot.com/instructions-set:cp)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addCpB();

  /***
   * @brief CP n compares register A with immediate value n by internally
   * subtracting n from A.
   * @note [CP](http://z80-heaven.wikidot.com/instructions-set:cp)
   * @param n Immediate 8-bit value to compare with A.
   * @result opcode size: 2 bytes; clock cycles: 7 t-states
   */
  void addCp(unsigned char n);

  /***
   * @brief INC A increments the value of register A by one.
   * @note [INC](http://z80-heaven.wikidot.com/instructions-set:inc)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addIncA();

  /***
   * @brief INC H increments the value of register H by one.
   * @note [INC](http://z80-heaven.wikidot.com/instructions-set:inc)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addIncH();

  /***
   * @brief INC L increments the value of register L by one.
   * @note [INC](http://z80-heaven.wikidot.com/instructions-set:inc)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addIncL();

  /***
   * @brief INC D increments the value of register D by one.
   * @note [INC](http://z80-heaven.wikidot.com/instructions-set:inc)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addIncD();

  /***
   * @brief INC HL increments the contents of register pair HL by one.
   * @note [INC](http://z80-heaven.wikidot.com/instructions-set:inc)
   * @result opcode size: 1 byte; clock cycles: 6 t-states
   */
  void addIncHL();

  /***
   * @brief INC DE increments the contents of register pair DE by one.
   * @note [INC](http://z80-heaven.wikidot.com/instructions-set:inc)
   * @result opcode size: 1 byte; clock cycles: 6 t-states
   */
  void addIncDE();

  /***
   * @brief DEC HL decrements the contents of register pair HL by one.
   * @note [DEC](http://z80-heaven.wikidot.com/instructions-set:dec)
   * @result opcode size: 1 byte; clock cycles: 6 t-states
   */
  void addDecHL();

  /***
   * @brief ADD A, n adds the immediate value n to register A.
   * @note [ADD](http://z80-heaven.wikidot.com/instructions-set:add)
   * @param n Immediate 8-bit value to add to A.
   * @result opcode size: 2 bytes; clock cycles: 7 t-states
   */
  void addAdd(unsigned char n);

  /***
   * @brief ADD A, H adds the value of register H to register A.
   * @note [ADD](http://z80-heaven.wikidot.com/instructions-set:add)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addAddH();

  /***
   * @brief ADD A, L adds the value of register L to register A.
   * @note [ADD](http://z80-heaven.wikidot.com/instructions-set:add)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addAddL();

  /***
   * @brief ADD A, A adds the value of register A to itself.
   * @note [ADD](http://z80-heaven.wikidot.com/instructions-set:add)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addAddA();

  /***
   * @brief ADD HL, HL adds the HL register pair to itself.
   * @note [ADD HL, rr](http://z80-heaven.wikidot.com/instructions-set:add)
   * @result opcode size: 1 byte; clock cycles: 11 t-states
   */
  void addAddHLHL();

  /***
   * @brief ADD HL, BC adds the BC register pair to HL.
   * @note [ADD HL, rr](http://z80-heaven.wikidot.com/instructions-set:add)
   * @result opcode size: 1 byte; clock cycles: 11 t-states
   */
  void addAddHLBC();

  /***
   * @brief ADD HL, DE adds the DE register pair to HL.
   * @note [ADD HL, rr](http://z80-heaven.wikidot.com/instructions-set:add)
   * @result opcode size: 1 byte; clock cycles: 11 t-states
   */
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
