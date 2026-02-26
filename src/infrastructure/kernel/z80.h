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
  //! @remark allocation size need to be at least 20*0xFFFF
  unsigned char* code;

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
   * @defgroup IntructionsOpcodeGroup
   * @brief Z80 instructions to opcode group
   * @{
   */

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

  /***
   * @brief ADC A, n adds the value of n and the carry flag to register A.
   * @note [ADC A, n](http://z80-heaven.wikidot.com/instructions-set:adc)
   * @param n Immediate 8-bit value to add with carry.
   * @result opcode size: 2 bytes; clock cycles: 7 t-states
   */
  void addAdcA(unsigned char n);

  /***
   * @brief SUB n subtracts the immediate value n from register A.
   * @note [SUB A, n](http://z80-heaven.wikidot.com/instructions-set:sub)
   * @param n Immediate 8-bit value to subtract from A.
   * @result opcode size: 2 bytes; clock cycles: 7 t-states
   */
  void addSub(unsigned char n);

  /***
   * @brief SBC HL, DE subtracts DE and the carry flag from HL.
   * @note [SBC HL, rr](http://z80-heaven.wikidot.com/instructions-set:sbc)
   * @result opcode size: 2 bytes; clock cycles: 15 t-states
   */
  void addSbcHLDE();

  /***
   * @brief SBC HL, BC subtracts BC and the carry flag from HL.
   * @note [SBC HL, rr](http://z80-heaven.wikidot.com/instructions-set:sbc)
   * @result opcode size: 2 bytes; clock cycles: 15 t-states
   */
  void addSbcHLBC();

  /***
   * @brief RRCA rotates the bits in register A right by one. Bit 0 is copied to
   * the carry flag and to bit 7.
   * @note [RRCA](http://z80-heaven.wikidot.com/instructions-set:rrca)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addRRCA();

  /***
   * @brief RLA rotates register A left through the carry flag.
   * @note [RLA](http://z80-heaven.wikidot.com/instructions-set:rla)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addRLA();

  /***
   * @brief SRA H shifts the contents of register H right with sign extension.
   * @note [SRA r](http://z80-heaven.wikidot.com/instructions-set:sra)
   * @result opcode size: 2 bytes; clock cycles: 8 t-states
   */
  void addSRAH();

  /***
   * @brief SRL H shifts the contents of register H right logically, inserting 0
   * into bit 7.
   * @note [SRL r](http://z80-heaven.wikidot.com/instructions-set:srl)
   * @result opcode size: 2 bytes; clock cycles: 8 t-states
   */
  void addSRLH();

  /***
   * @brief RR L rotates the bits in register L right through the carry flag.
   * @note [RR r](http://z80-heaven.wikidot.com/instructions-set:rr)
   * @result opcode size: 2 bytes; clock cycles: 8 t-states
   */
  void addRRL();

  /***
   * @brief RRA rotates register A right through the carry flag.
   * @note [RRA](http://z80-heaven.wikidot.com/instructions-set:rra)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addRRA();

  /***
   * @brief CPL inverts all bits in register A (complement A).
   * @note [CPL](http://z80-heaven.wikidot.com/instructions-set:cpl)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addCPL();

  /***
   * @brief LDI loads the byte at (HL) into (DE) and increments HL, DE, and
   * decrements BC.
   * @note [LDI](http://z80-heaven.wikidot.com/instructions-set:ldi)
   * @result opcode size: 2 bytes; clock cycles: 16 t-states
   */
  void addLDI();

  /***
   * @brief LDIR repeats the LDI instruction until BC is zero.
   * @note [LDIR](http://z80-heaven.wikidot.com/instructions-set:ldir)
   * @result opcode size: 2 bytes; clock cycles: 16 t-states per iteration
   */
  void addLDIR();

  /***
   * @brief DEC A decrements the value in register A by 1.
   * @note [DEC r](http://z80-heaven.wikidot.com/instructions-set:dec)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addDecA();

  /***
   * @brief DEC E decrements the value in register E by 1.
   * @note [DEC r](http://z80-heaven.wikidot.com/instructions-set:dec)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addDecE();

  /***
   * @brief RET returns from a subroutine by popping the top of the stack into
   * the PC.
   * @note [RET](http://z80-heaven.wikidot.com/instructions-set:ret)
   * @result opcode size: 1 byte; clock cycles: 10 t-states
   */
  void addRet();

  /***
   * @brief RET C returns from subroutine if the carry flag is set.
   * @note [RET cc](http://z80-heaven.wikidot.com/instructions-set:ret)
   * @result opcode size: 1 byte; clock cycles: 5 (if not taken) or 11 (if
   * taken) t-states
   */
  void addRetC();

  /***
   * @brief LD A, n loads the immediate 8-bit value into register A.
   * @note [LD r, n](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @param n Immediate 8-bit value to load into A.
   * @result opcode size: 2 bytes; clock cycles: 7 t-states
   */
  void addLdA(unsigned char n);

  /***
   * @brief LD A, B loads the contents of register B into register A.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdAB();

  /***
   * @brief LD A, C loads the contents of register C into register A.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdAC();

  /***
   * @brief LD A, H loads the contents of register H into register A.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdAH();

  /***
   * @brief LD A, L loads the contents of register L into register A.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdAL();

  /***
   * @brief LD A, E loads the contents of register E into register A.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdAE();

  /***
   * @brief LD A, D loads the contents of register D into register A.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdAD();

  /***
   * @brief LD A, (HL) loads the byte at the memory address pointed by HL into
   * register A.
   * @note [LD A, (HL)](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 7 t-states
   */
  void addLdAiHL();

  /***
   * @brief LD A, (DE) loads the byte at the memory address pointed by DE into
   * register A.
   * @note [LD A, (DE)](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 7 t-states
   */
  void addLdAiDE();

  /***
   * @brief LD H, A loads the contents of register A into register H.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdHA();

  /***
   * @brief LD L, A loads the contents of register A into register L.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdLA();

  /***
   * @brief LD L, C loads the contents of register C into register L.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdLC();

  /***
   * @brief LD L, E loads the contents of register E into register L.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdLE();

  /***
   * @brief LD L, H loads the contents of register H into register L.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdLH();

  /***
   * @brief LD L, n loads the 8-bit immediate value into register L.
   * @param n The 8-bit immediate value to be loaded.
   * @note [LD r, n](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 2 bytes; clock cycles: 7 t-states
   */
  void addLdL(unsigned char n);

  /***
   * @brief LD H, E loads the contents of register E into register H.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdHE();

  /***
   * @brief LD H, C loads the contents of register C into register H.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdHC();

  /***
   * @brief LD H, D loads the contents of register D into register H.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdHD();

  /***
   * @brief LD H, L loads the contents of register L into register H.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdHL();

  /***
   * @brief LD H, n loads the 8-bit immediate value into register H.
   * @param n The 8-bit immediate value to be loaded.
   * @note [LD r, n](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 2 bytes; clock cycles: 7 t-states
   */
  void addLdH(unsigned char n);

  /***
   * @brief LD L, (HL) loads into register L the value from the memory address
   * pointed to by HL.
   * @note [LD r, (HL)](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 7 t-states
   */
  void addLdLiHL();

  /***
   * @brief LD B, A loads the contents of register A into register B.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdBA();

  /***
   * @brief LD B, H loads the contents of register H into register B.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdBH();

  /***
   * @brief LD B, L loads the contents of register L into register B.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdBL();

  /***
   * @brief LD B, E loads the contents of register E into register B.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdBE();

  /***
   * @brief LD B, (HL) loads into register B the value from the memory address
   * pointed to by HL.
   * @note [LD r, (HL)](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 7 t-states
   */
  void addLdBiHL();

  /***
   * @brief LD B, n loads the 8-bit immediate value into register B.
   * @param n The 8-bit immediate value to be loaded.
   * @note [LD r, n](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 2 bytes; clock cycles: 7 t-states
   */
  void addLdB(unsigned char n);

  /***
   * @brief LD C, n loads the 8-bit immediate value into register C.
   * @param n The 8-bit immediate value to be loaded.
   * @note [LD r, n](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 2 bytes; clock cycles: 7 t-states
   */
  void addLdC(unsigned char n);

  /***
   * @brief LD E, (HL) loads into register E the value from the memory address
   * pointed to by HL.
   * @note [LD r, (HL)](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 7 t-states
   */
  void addLdEiHL();

  /***
   * @brief LD E, A loads the contents of register A into register E.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdEA();

  /***
   * @brief LD E, L loads the contents of register L into register E.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdEL();

  /***
   * @brief LD C, A loads the contents of register A into register C.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdCA();

  /***
   * @brief LD C, B loads the contents of register B into register C.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdCB();

  /***
   * @brief LD C, E loads the contents of register E into register C.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdCE();

  /***
   * @brief LD C, D loads the contents of register D into register C.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdCD();

  /***
   * @brief LD C, H loads the contents of register H into register C.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdCH();

  /***
   * @brief LD C, L loads the contents of register L into register C.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdCL();

  /***
   * @brief LD C, (HL) loads into register C the value from the memory address
   * pointed to by HL.
   * @note [LD r, (HL)](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 7 t-states
   */
  void addLdCiHL();

  /***
   * @brief LD D, A loads the contents of register A into register D.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdDA();

  /***
   * @brief LD D, L loads the contents of register L into register D.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdDL();

  /***
   * @brief LD D, H loads the contents of register H into register D.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdDH();

  /***
   * @brief LD D, (HL) loads into register D the value from the memory address
   * pointed to by HL.
   * @note [LD r, (HL)](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 7 t-states
   */
  void addLdDiHL();

  /***
   * @brief LD H, B loads the contents of register B into register H.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdHB();

  /***
   * @brief LD HL, n sets the HL register pair to the immediate 16-bit value n.
   * @param n The 16-bit immediate value to be loaded.
   * @note [LD rr, nn](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 3 bytes; clock cycles: 10 t-states
   */
  void addLdHL(int n);

  /***
   * @brief LD BC, n sets the BC register pair to the immediate 16-bit value n.
   * @param n The 16-bit immediate value to be loaded.
   * @note [LD rr, nn](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 3 bytes; clock cycles: 10 t-states
   */
  void addLdBC(int n);

  /***
   * @brief LD D, E loads the contents of register E into register D.
   * @note [LD r, r'](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 4 t-states
   */
  void addLdDE();

  /***
   * @brief LD DE, n sets the DE register pair to the immediate 16-bit value n.
   * @param n The 16-bit immediate value to be loaded.
   * @note [LD rr, nn](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 3 bytes; clock cycles: 10 t-states
   */
  void addLdDE(int n);

  /***
   * @brief LD IX, n sets the IX register pair to the immediate 16-bit value n.
   * @param n The 16-bit immediate value to be loaded.
   * @note [LD IX, nn](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 4 bytes; clock cycles: 14 t-states
   */
  void addLdIX(int n);

  /***
   * @brief LD A, (ii) loads the value at memory address ii into register A.
   * @param i The 16-bit address.
   * @note [LD A, (nn)](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 3 bytes; clock cycles: 13 t-states
   */
  void addLdAii(int i);

  /***
   * @brief LD HL, (ii) loads the 16-bit value at memory address ii into HL.
   * @param i The 16-bit address.
   * @note [LD rr, (nn)](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 3 bytes; clock cycles: 16 t-states
   */
  void addLdHLii(int i);

  /***
   * @brief LD BC, (ii) loads the 16-bit value at memory address ii into BC.
   * @param i The 16-bit address.
   * @note [LD rr, (nn)](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 3 bytes; clock cycles: 16 t-states
   */
  void addLdBCii(int i);

  /***
   * @brief LD DE, (ii) loads the 16-bit value at memory address ii into DE.
   * @param i The 16-bit address.
   * @note [LD rr, (nn)](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 3 bytes; clock cycles: 16 t-states
   */
  void addLdDEii(int i);

  /***
   * @brief LD IX, (ii) loads the 16-bit value at memory address ii into IX.
   * @param i The 16-bit address.
   * @note [LD IX, (nn)](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 4 bytes; clock cycles: 20 t-states
   */
  void addLdIXii(int i);

  /***
   * @brief LD IY, (ii) loads the 16-bit value at memory address ii into IY.
   * @param i The 16-bit address.
   * @note [LD IY, (nn)](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 4 bytes; clock cycles: 20 t-states
   */
  void addLdIYii(int i);

  /***
   * @brief LD (ii), A stores the contents of register A at memory address ii.
   * @param i The 16-bit address.
   * @note [LD (nn), A](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 3 bytes; clock cycles: 13 t-states
   */
  void addLdiiA(int i);

  /***
   * @brief LD (ii), HL stores the contents of register pair HL at memory
   * address ii.
   * @param i The 16-bit address.
   * @note [LD (nn), rr](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 3 bytes; clock cycles: 16 t-states
   */
  void addLdiiHL(int i);

  /***
   * @brief LD (ii), DE stores the contents of register pair DE at memory
   * address ii.
   * @param i The 16-bit address.
   * @note [LD (nn), rr](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 3 bytes; clock cycles: 16 t-states
   */
  void addLdiiDE(int i);

  /***
   * @brief LD (ii), SP stores the contents of the stack pointer at memory
   * address ii.
   * @param i The 16-bit address.
   * @note [LD (nn), SP](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 3 bytes; clock cycles: 20 t-states
   */
  void addLdiiSP(int i);

  /***
   * @brief LD SP, (ii) loads the 16-bit value at memory address ii into the SP
   * register.
   * @param i The 16-bit address.
   * @note [LD SP, (nn)](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 3 bytes; clock cycles: 20 t-states
   */
  void addLdSPii(int i);

  /***
   * @brief LD SP, HL sets the stack pointer to the value of register pair HL.
   * @note [LD SP, HL](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 6 t-states
   */
  void addLdSPHL();

  /***
   * @brief LD (HL), n stores the 8-bit value n at the memory location pointed
   * to by HL.
   * @param n The 8-bit immediate value to be loaded.
   * @note [LD (HL), n](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 2 bytes; clock cycles: 10 t-states
   */
  void addLdiHL(unsigned char n);

  /***
   * @brief LD (HL), A stores the contents of register A into the memory address
   * pointed to by HL.
   * @note [LD (HL), r](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 7 t-states
   */
  void addLdiHLA();

  /***
   * @brief LD (HL), B stores the contents of register B into the memory address
   * pointed to by HL.
   * @note [LD (HL), r](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 7 t-states
   */
  void addLdiHLB();

  /***
   * @brief LD (HL), C stores the contents of register C into the memory address
   * pointed to by HL.
   * @note [LD (HL), r](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 7 t-states
   */
  void addLdiHLC();

  /***
   * @brief LD (HL), D stores the contents of register D into the memory address
   * pointed to by HL.
   * @note [LD (HL), r](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 7 t-states
   */
  void addLdiHLD();

  /***
   * @brief LD (HL), E stores the contents of register E into the memory address
   * pointed to by HL.
   * @note [LD (HL), r](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 7 t-states
   */
  void addLdiHLE();

  /***
   * @brief LD (DE), A stores the contents of register A into the memory address
   * pointed to by DE.
   * @note [LD (DE), A](http://z80-heaven.wikidot.com/instructions-set:ld)
   * @result opcode size: 1 byte; clock cycles: 7 t-states
   */
  void addLdiDEA();

  /***
   * @brief JR n jumps relative to the current instruction pointer by n bytes.
   *        The offset is a signed 8-bit value, allowing jumps -128 to +127.
   * @param n The 8-bit immediate value to be jumped (unsigned: 0xFF = -1) .
   * @note [JR](http://z80-heaven.wikidot.com/instructions-set:jr)
   * @result opcode size: 2 bytes; clock cycles: 12 t-states (if taken), 7
   * t-states (if not)
   */
  void addJr(unsigned char n);

  /***
   * @brief JR Z, n jumps relative to the current instruction pointer by n bytes
   * if the Zero flag is set. The offset is a signed 8-bit value.
   * @param n The 8-bit immediate value to be jumped (unsigned: 0xFF = -1) .
   * @note [JR Z, n](http://z80-heaven.wikidot.com/instructions-set:jr)
   * @result opcode size: 2 bytes; clock cycles: 12 t-states (if taken), 7
   * t-states (if not)
   */
  void addJrZ(unsigned char n);

  /***
   * @brief JR NZ, n jumps relative to the current instruction pointer by n
   * bytes if the Zero flag is not set. The offset is a signed 8-bit value.
   * @param n The 8-bit immediate value to be jumped (unsigned: 0xFF = -1) .
   * @note [JR NZ, n](http://z80-heaven.wikidot.com/instructions-set:jr)
   * @result opcode size: 2 bytes; clock cycles: 12 t-states (if taken), 7
   * t-states (if not)
   */
  void addJrNZ(unsigned char n);

  /***
   * @brief JR C, n jumps relative to the current instruction pointer by n bytes
   * if the Carry flag is set. The offset is a signed 8-bit value.
   * @param n The 8-bit immediate value to be jumped (unsigned: 0xFF = -1) .
   * @note [JR C, n](http://z80-heaven.wikidot.com/instructions-set:jr)
   * @result opcode size: 2 bytes; clock cycles: 12 t-states (if taken), 7
   * t-states (if not)
   */
  void addJrC(unsigned char n);

  /***
   * @brief JR NC, n jumps relative to the current instruction pointer by n
   * bytes if the Carry flag is not set. The offset is a signed 8-bit value.
   * @param n The 8-bit immediate value to be jumped (unsigned: 0xFF = -1) .
   * @note [JR NC, n](http://z80-heaven.wikidot.com/instructions-set:jr)
   * @result opcode size: 2 bytes; clock cycles: 12 t-states (if taken), 7
   * t-states (if not)
   */
  void addJrNC(unsigned char n);

  /***
   * @brief JP nn performs an absolute jump to the specified 16-bit address.
   * @param n The 16-bit address to jump.
   * @note [JP](http://z80-heaven.wikidot.com/instructions-set:jp)
   * @result opcode size: 3 bytes; clock cycles: 10 t-states
   */
  void addJp(int n);

  /***
   * @brief JP Z, nn performs an absolute jump to the specified 16-bit address
   * if the Zero flag is set.
   * @param n The 16-bit address to jump.
   * @note [JP Z, nn](http://z80-heaven.wikidot.com/instructions-set:jp)
   * @result opcode size: 3 bytes; clock cycles: 10 t-states (if taken), 10
   * t-states (if not)
   */
  void addJpZ(int n);

  /***
   * @brief JP NZ, nn performs an absolute jump to the specified 16-bit address
   * if the Zero flag is not set.
   * @param n The 16-bit address to jump.
   * @note [JP NZ, nn](http://z80-heaven.wikidot.com/instructions-set:jp)
   * @result opcode size: 3 bytes; clock cycles: 10 t-states (if taken), 10
   * t-states (if not)
   */
  void addJpNZ(int n);

  /***
   * @brief CALL nn calls a subroutine located at the specified 16-bit address.
   *        The current program counter is pushed to the stack before the jump.
   * @param word The 16-bit subroutine address to call.
   * @note [CALL](http://z80-heaven.wikidot.com/instructions-set:call)
   * @result opcode size: 3 bytes; clock cycles: 17 t-states
   */
  void addCall(unsigned int word);

  /***
   * @remark End of IntructionsOpcodeGroup
   * @}
   */

 public:
  IZ80();

  int code_size;
};

#endif  // Z80_H
