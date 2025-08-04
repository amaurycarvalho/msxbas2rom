/***
 * @file z80.cpp
 * @brief Z80 interface class implementation (bridge design pattern),
 *        specialized as a Z80 code stream writer for MSX system
 * @author Amaury Carvalho (2025)
 * @note
 *   https://refactoring.guru/design-patterns/bridge
 *   Z80 Opcodes:
 *     http://z80-heaven.wikidot.com/instructions-set
 *     https://clrhome.org/table/
 */

#include "z80.h"

/***
 * @name IZ80 interface class code
 */

IZ80::IZ80() {
  code_size = 0;
  code_pointer = 0;
  code = 0;
}

void IZ80::pushLastCode() {
  if (code) {
    int i;
    for (i = 4; i > 0; i--) {
      last_code[i] = last_code[i - 1];
    }
    last_code[0] = &code[code_pointer];
  }
}

void IZ80::popLastCode() {
  int i;
  for (i = 1; i < 5; i++) {
    last_code[i - 1] = last_code[i];
  }
}

void IZ80::addCodeByte(unsigned char byte) {
  if (code) {
    code[code_pointer++] = byte;
    code_size++;
  }
}

void IZ80::addByte(unsigned char byte) {
  pushLastCode();
  addCodeByte(byte);
}

void IZ80::addWord(unsigned int word) {
  pushLastCode();
  addCodeByte(word & 0xff);
  addCodeByte((word >> 8) & 0xff);
}

void IZ80::addWord(unsigned char byte1, unsigned char byte2) {
  pushLastCode();
  addCodeByte(byte1);
  addCodeByte(byte2);
}

void IZ80::addCmd(unsigned char byte, unsigned int word) {
  pushLastCode();
  addCodeByte(byte);
  addCodeByte(word & 0xff);
  addCodeByte((word >> 8) & 0xff);
}

void IZ80::addJr(unsigned char n) {
  addWord(0x18, n);
}

void IZ80::addJrZ(unsigned char n) {
  addWord(0x28, n);
}

void IZ80::addJrNZ(unsigned char n) {
  addWord(0x20, n);
}

void IZ80::addJrC(unsigned char n) {
  addWord(0x38, n);
}

void IZ80::addJrNC(unsigned char n) {
  addWord(0x30, n);
}

void IZ80::addJp(int n) {
  addCmd(0xC3, n);
}

void IZ80::addJpZ(int n) {
  addCmd(0xCA, n);
}

void IZ80::addJpNZ(int n) {
  addCmd(0xC2, n);
}

void IZ80::addCall(unsigned int address) {
  addCmd(0xCD, address);
}

void IZ80::addNop() {
  addByte(0x00);
}

void IZ80::addExAF() {
  addByte(0x08);
}

void IZ80::addExDEHL() {
  addByte(0xEB);
}

void IZ80::addExiSPHL() {
  addByte(0xE3);
}

void IZ80::addExx() {
  addByte(0xD9);
}

void IZ80::addEI() {
  addByte(0xFB);
}

void IZ80::addDI() {
  addByte(0xF3);
}

void IZ80::addPushAF() {
  addByte(0xF5);
}

void IZ80::addPushHL() {
  addByte(0xE5);
}

void IZ80::addPushBC() {
  addByte(0xC5);
}

void IZ80::addPushDE() {
  addByte(0xD5);
}

void IZ80::addPopAF() {
  addByte(0xF1);
}

void IZ80::addPopHL() {
  addByte(0xE1);
}

void IZ80::addPopBC() {
  addByte(0xC1);
}

void IZ80::addPopDE() {
  addByte(0xD1);
}

void IZ80::addPopIX() {
  addWord(0xDD, 0xE1);
}

void IZ80::addPopIY() {
  addWord(0xFD, 0xE1);
}

void IZ80::addXorA() {
  addByte(0xAF);
}

void IZ80::addXorH() {
  addByte(0xAC);
}

void IZ80::addXorE() {
  addByte(0xAB);
}

void IZ80::addXorD() {
  addByte(0xAA);
}

void IZ80::addAndA() {
  addByte(0xA7);
}

void IZ80::addAndD() {
  addByte(0xA2);
}

void IZ80::addAnd(unsigned char n) {
  addWord(0xE6, n);
}

void IZ80::addOr(unsigned char n) {
  addWord(0xF6, n);
}

void IZ80::addOrH() {
  addByte(0xB4);
}

void IZ80::addOrL() {
  addByte(0xB5);
}

void IZ80::addOrB() {
  addByte(0xB0);
}

void IZ80::addOrC() {
  addByte(0xB1);
}

void IZ80::addOrE() {
  addByte(0xB3);
}

void IZ80::addCpL() {
  addByte(0xBD);
}

void IZ80::addCpB() {
  addByte(0xB8);
}

void IZ80::addCp(unsigned char n) {
  addWord(0xFE, n);
}

void IZ80::addIncA() {
  addByte(0x3C);
}

void IZ80::addIncH() {
  addByte(0x24);
}

void IZ80::addIncL() {
  addByte(0x2C);
}

void IZ80::addIncD() {
  addByte(0x14);
}

void IZ80::addIncHL() {
  addByte(0x23);
}

void IZ80::addIncDE() {
  addByte(0x13);
}

void IZ80::addDecHL() {
  addByte(0x2B);
}

void IZ80::addAdd(unsigned char n) {
  addWord(0xC6, n);
}

void IZ80::addAddH() {
  addByte(0x7C);
}

void IZ80::addAddL() {
  addByte(0x85);
}

void IZ80::addAddA() {
  addByte(0x87);
}

void IZ80::addAdcA(unsigned char n) {
  addWord(0xCE, n);
}

void IZ80::addAddHLHL() {
  addByte(0x29);
}

void IZ80::addAddHLBC() {
  addByte(0x09);
}

void IZ80::addAddHLDE() {
  addByte(0x19);
}

void IZ80::addSub(unsigned char n) {
  addWord(0xD6, n);
}

void IZ80::addSbcHLDE() {
  addWord(0xED, 0x52);
}

void IZ80::addSbcHLBC() {
  addWord(0xED, 0x42);
}

void IZ80::addDecA() {
  addByte(0x3D);
}

void IZ80::addDecE() {
  addByte(0x1D);
}

void IZ80::addRRCA() {
  addByte(0x0F);
}

void IZ80::addRLA() {
  addByte(0x17);
}

void IZ80::addSRAH() {
  addWord(0xCB, 0x2C);
}

void IZ80::addSRLH() {
  addWord(0xCB, 0x3C);
}

void IZ80::addRRL() {
  addWord(0xCB, 0x1D);
}

void IZ80::addRRA() {
  addByte(0x1F);
}

void IZ80::addCPL() {
  addByte(0x2F);
}

void IZ80::addLDI() {
  addWord(0xED, 0xA0);
}

void IZ80::addLDIR() {
  addWord(0xED, 0xB0);
}

void IZ80::addRet() {
  addByte(0xC9);
}

void IZ80::addRetC() {
  addByte(0xD8);
}

void IZ80::addLdA(unsigned char n) {
  addWord(0x3E, n);
}

void IZ80::addLdAB() {
  addByte(0x78);
}

void IZ80::addLdAC() {
  addByte(0x79);
}

void IZ80::addLdAH() {
  addByte(0x7C);
}

void IZ80::addLdAL() {
  addByte(0x7D);
}

void IZ80::addLdAE() {
  addByte(0x7B);
}

void IZ80::addLdAD() {
  addByte(0x7A);
}

void IZ80::addLdAiHL() {
  addByte(0x7E);
}

void IZ80::addLdAiDE() {
  addByte(0x1A);
}

void IZ80::addLdHA() {
  addByte(0x67);
}

void IZ80::addLdLA() {
  addByte(0x6F);
}

void IZ80::addLdL(unsigned char n) {
  addWord(0x2E, n);
}

void IZ80::addLdHE() {
  addByte(0x63);
}

void IZ80::addLdHC() {
  addByte(0x61);
}

void IZ80::addLdHL() {
  addByte(0x65);
}

void IZ80::addLdH(unsigned char n) {
  addWord(0x26, n);
}

void IZ80::addLdLiHL() {
  addByte(0x6E);
}

void IZ80::addLdBA() {
  addByte(0x47);
}

void IZ80::addLdBH() {
  addByte(0x44);
}

void IZ80::addLdBL() {
  addByte(0x45);
}

void IZ80::addLdBE() {
  addByte(0x43);
}

void IZ80::addLdBiHL() {
  addByte(0x46);
}

void IZ80::addLdB(unsigned char n) {
  addWord(0x06, n);
}

void IZ80::addLdC(unsigned char n) {
  addWord(0x0E, n);
}

void IZ80::addLdEiHL() {
  addByte(0x5E);
}

void IZ80::addLdEA() {
  addByte(0x5F);
}

void IZ80::addLdEL() {
  addByte(0x5D);
}

void IZ80::addLdCA() {
  addByte(0x4F);
}

void IZ80::addLdCB() {
  addByte(0x48);
}

void IZ80::addLdCD() {
  addByte(0x4A);
}

void IZ80::addLdCE() {
  addByte(0x4B);
}

void IZ80::addLdCH() {
  addByte(0x4C);
}

void IZ80::addLdCL() {
  addByte(0x4D);
}

void IZ80::addLdCiHL() {
  addByte(0x4E);
}

void IZ80::addLdDA() {
  addByte(0x57);
}

void IZ80::addLdDL() {
  addByte(0x55);
}

void IZ80::addLdDH() {
  addByte(0x54);
}

void IZ80::addLdDiHL() {
  addByte(0x56);
}

void IZ80::addLdHB() {
  addByte(0x60);
}

void IZ80::addLdLC() {
  addByte(0x69);
}

void IZ80::addLdLH() {
  addByte(0x6C);
}

void IZ80::addLdHL(int n) {
  addCmd(0x21, n);
}

void IZ80::addLdHLmegarom() {
  addCmd(0xFF, 0x0000);
}

void IZ80::addLdBC(int n) {
  addCmd(0x01, n);
}

void IZ80::addLdDE() {
  addByte(0x53);
}

void IZ80::addLdDE(int n) {
  addCmd(0x11, n);
}

void IZ80::addLdIX(int n) {
  addByte(0xDD);
  addCmd(0x21, n);
}

void IZ80::addLdAii(int i) {
  addCmd(0x3A, i);
}

void IZ80::addLdHLii(int i) {
  addCmd(0x2A, i);
}

void IZ80::addLdBCii(int i) {
  addByte(0xED);
  addCmd(0x4B, i);
}

void IZ80::addLdDEii(int i) {
  addByte(0xED);
  addCmd(0x5B, i);
}

void IZ80::addLdIXii(int i) {
  // addWord(0xDD,0xED);
  // addCmd(0x5B, i);
  addByte(0xDD);
  addLdHLii(i);
}

void IZ80::addLdIYii(int i) {
  addByte(0xFD);
  addLdHLii(i);
}

void IZ80::addLdiiA(int i) {
  addCmd(0x32, i);
}

void IZ80::addLdiiHL(int i) {
  addCmd(0x22, i);
}

void IZ80::addLdiiDE(int i) {
  addByte(0xED);
  addCmd(0x53, i);
}

void IZ80::addLdiiSP(int i) {
  addByte(0xED);
  addCmd(0x73, i);
}

void IZ80::addLdSPii(int i) {
  addByte(0xED);
  addCmd(0x7b, i);
}

void IZ80::addLdSPHL() {
  addByte(0xF9);
}

void IZ80::addLdiHL(unsigned char n) {
  addWord(0x36, n);
}

void IZ80::addLdiHLA() {
  addByte(0x77);
}

void IZ80::addLdiHLB() {
  addByte(0x70);
}

void IZ80::addLdiHLC() {
  addByte(0x71);
}

void IZ80::addLdiHLD() {
  addByte(0x72);
}

void IZ80::addLdiHLE() {
  addByte(0x73);
}

void IZ80::addLdiDEA() {
  addByte(0x12);
}
