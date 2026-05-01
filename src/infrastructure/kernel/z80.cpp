/***
 * @file z80.cpp
 * @brief Z80 interface class implementation (bridge design pattern),
 *        specialized as a Z80 code stream writer for MSX system
 * @author Amaury Carvalho (2025)
 * @note
 *   Z80 Opcodes:
 *     http://z80-heaven.wikidot.com/instructions-set
 *     https://clrhome.org/table/
 */

#include "z80.h"

/***
 * @name ICpuOpcodeWriter interface class code
 */

void Z80OpcodeWriter::addJr(unsigned char n) {
  addWord(0x18, n);
}

void Z80OpcodeWriter::addJrZ(unsigned char n) {
  addWord(0x28, n);
}

void Z80OpcodeWriter::addJrNZ(unsigned char n) {
  addWord(0x20, n);
}

void Z80OpcodeWriter::addJrC(unsigned char n) {
  addWord(0x38, n);
}

void Z80OpcodeWriter::addJrNC(unsigned char n) {
  addWord(0x30, n);
}

void Z80OpcodeWriter::addJp(int n) {
  addCmd(0xC3, n);
}

void Z80OpcodeWriter::addJpC(int n) {
  addCmd(0xDA, n);
}

void Z80OpcodeWriter::addJpNC(int n) {
  addCmd(0xD2, n);
}

void Z80OpcodeWriter::addJpZ(int n) {
  addCmd(0xCA, n);
}

void Z80OpcodeWriter::addJpNZ(int n) {
  addCmd(0xC2, n);
}

void Z80OpcodeWriter::addCall(unsigned int address) {
  addCmd(0xCD, address);
}

void Z80OpcodeWriter::addCallZ(unsigned int address) {
  addCmd(0xCC, address);
}

void Z80OpcodeWriter::addCallNZ(unsigned int address) {
  addCmd(0xC4, address);
}

void Z80OpcodeWriter::addCallC(unsigned int address) {
  addCmd(0xDC, address);
}

void Z80OpcodeWriter::addCallNC(unsigned int address) {
  addCmd(0xD4, address);
}

void Z80OpcodeWriter::addNop() {
  addByte(0x00);
}

void Z80OpcodeWriter::addExAF() {
  addByte(0x08);
}

void Z80OpcodeWriter::addExDEHL() {
  addByte(0xEB);
}

void Z80OpcodeWriter::addExiSPHL() {
  addByte(0xE3);
}

void Z80OpcodeWriter::addExx() {
  addByte(0xD9);
}

void Z80OpcodeWriter::addEI() {
  addByte(0xFB);
}

void Z80OpcodeWriter::addDI() {
  addByte(0xF3);
}

void Z80OpcodeWriter::addPushAF() {
  addByte(0xF5);
}

void Z80OpcodeWriter::addPushHL() {
  addByte(0xE5);
}

void Z80OpcodeWriter::addPushBC() {
  addByte(0xC5);
}

void Z80OpcodeWriter::addPushDE() {
  addByte(0xD5);
}

void Z80OpcodeWriter::addPopAF() {
  addByte(0xF1);
}

void Z80OpcodeWriter::addPopHL() {
  addByte(0xE1);
}

void Z80OpcodeWriter::addPopBC() {
  addByte(0xC1);
}

void Z80OpcodeWriter::addPopDE() {
  addByte(0xD1);
}

void Z80OpcodeWriter::addPopIX() {
  addWord(0xDD, 0xE1);
}

void Z80OpcodeWriter::addPopIY() {
  addWord(0xFD, 0xE1);
}

void Z80OpcodeWriter::addXorA() {
  addByte(0xAF);
}

void Z80OpcodeWriter::addXorH() {
  addByte(0xAC);
}

void Z80OpcodeWriter::addXorE() {
  addByte(0xAB);
}

void Z80OpcodeWriter::addXorD() {
  addByte(0xAA);
}

void Z80OpcodeWriter::addAndA() {
  addByte(0xA7);
}

void Z80OpcodeWriter::addAndD() {
  addByte(0xA2);
}

void Z80OpcodeWriter::addAnd(unsigned char n) {
  addWord(0xE6, n);
}

void Z80OpcodeWriter::addOr(unsigned char n) {
  addWord(0xF6, n);
}

void Z80OpcodeWriter::addOrH() {
  addByte(0xB4);
}

void Z80OpcodeWriter::addOrL() {
  addByte(0xB5);
}

void Z80OpcodeWriter::addOrB() {
  addByte(0xB0);
}

void Z80OpcodeWriter::addOrC() {
  addByte(0xB1);
}

void Z80OpcodeWriter::addOrE() {
  addByte(0xB3);
}

void Z80OpcodeWriter::addNeg() {
  addByte(0xED);
  addByte(0x44);
}

void Z80OpcodeWriter::addCpL() {
  addByte(0xBD);
}

void Z80OpcodeWriter::addCpB() {
  addByte(0xB8);
}

void Z80OpcodeWriter::addCp(unsigned char n) {
  addWord(0xFE, n);
}

void Z80OpcodeWriter::addIncA() {
  addByte(0x3C);
}

void Z80OpcodeWriter::addIncH() {
  addByte(0x24);
}

void Z80OpcodeWriter::addIncL() {
  addByte(0x2C);
}

void Z80OpcodeWriter::addIncD() {
  addByte(0x14);
}

void Z80OpcodeWriter::addIncHL() {
  addByte(0x23);
}

void Z80OpcodeWriter::addIncDE() {
  addByte(0x13);
}

void Z80OpcodeWriter::addDecHL() {
  addByte(0x2B);
}

void Z80OpcodeWriter::addAdd(unsigned char n) {
  addWord(0xC6, n);
}

void Z80OpcodeWriter::addAddH() {
  addByte(0x7C);
}

void Z80OpcodeWriter::addAddL() {
  addByte(0x85);
}

void Z80OpcodeWriter::addAddA() {
  addByte(0x87);
}

void Z80OpcodeWriter::addAdcA(unsigned char n) {
  addWord(0xCE, n);
}

void Z80OpcodeWriter::addAddHLHL() {
  addByte(0x29);
}

void Z80OpcodeWriter::addAddHLBC() {
  addByte(0x09);
}

void Z80OpcodeWriter::addAddHLDE() {
  addByte(0x19);
}

void Z80OpcodeWriter::addSub(unsigned char n) {
  addWord(0xD6, n);
}

void Z80OpcodeWriter::addSbcHLDE() {
  addWord(0xED, 0x52);
}

void Z80OpcodeWriter::addSbcHLBC() {
  addWord(0xED, 0x42);
}

void Z80OpcodeWriter::addDecA() {
  addByte(0x3D);
}

void Z80OpcodeWriter::addDecE() {
  addByte(0x1D);
}

void Z80OpcodeWriter::addRRCA() {
  addByte(0x0F);
}

void Z80OpcodeWriter::addRLA() {
  addByte(0x17);
}

void Z80OpcodeWriter::addSRAH() {
  addWord(0xCB, 0x2C);
}

void Z80OpcodeWriter::addSRLH() {
  addWord(0xCB, 0x3C);
}

void Z80OpcodeWriter::addRRL() {
  addWord(0xCB, 0x1D);
}

void Z80OpcodeWriter::addRRA() {
  addByte(0x1F);
}

void Z80OpcodeWriter::addCPL() {
  addByte(0x2F);
}

void Z80OpcodeWriter::addLDI() {
  addWord(0xED, 0xA0);
}

void Z80OpcodeWriter::addLDIR() {
  addWord(0xED, 0xB0);
}

void Z80OpcodeWriter::addRet() {
  addByte(0xC9);
}

void Z80OpcodeWriter::addRetC() {
  addByte(0xD8);
}

void Z80OpcodeWriter::addLdA(unsigned char n) {
  addWord(0x3E, n);
}

void Z80OpcodeWriter::addLdAB() {
  addByte(0x78);
}

void Z80OpcodeWriter::addLdAC() {
  addByte(0x79);
}

void Z80OpcodeWriter::addLdAH() {
  addByte(0x7C);
}

void Z80OpcodeWriter::addLdAL() {
  addByte(0x7D);
}

void Z80OpcodeWriter::addLdAE() {
  addByte(0x7B);
}

void Z80OpcodeWriter::addLdAD() {
  addByte(0x7A);
}

void Z80OpcodeWriter::addLdAiHL() {
  addByte(0x7E);
}

void Z80OpcodeWriter::addLdAiDE() {
  addByte(0x1A);
}

void Z80OpcodeWriter::addLdHA() {
  addByte(0x67);
}

void Z80OpcodeWriter::addLdLA() {
  addByte(0x6F);
}

void Z80OpcodeWriter::addLdL(unsigned char n) {
  addWord(0x2E, n);
}

void Z80OpcodeWriter::addLdHE() {
  addByte(0x63);
}

void Z80OpcodeWriter::addLdHC() {
  addByte(0x61);
}

void Z80OpcodeWriter::addLdHL() {
  addByte(0x65);
}

void Z80OpcodeWriter::addLdH(unsigned char n) {
  addWord(0x26, n);
}

void Z80OpcodeWriter::addLdLiHL() {
  addByte(0x6E);
}

void Z80OpcodeWriter::addLdBA() {
  addByte(0x47);
}

void Z80OpcodeWriter::addLdBH() {
  addByte(0x44);
}

void Z80OpcodeWriter::addLdBL() {
  addByte(0x45);
}

void Z80OpcodeWriter::addLdBE() {
  addByte(0x43);
}

void Z80OpcodeWriter::addLdBiHL() {
  addByte(0x46);
}

void Z80OpcodeWriter::addLdB(unsigned char n) {
  addWord(0x06, n);
}

void Z80OpcodeWriter::addLdC(unsigned char n) {
  addWord(0x0E, n);
}

void Z80OpcodeWriter::addLdEiHL() {
  addByte(0x5E);
}

void Z80OpcodeWriter::addLdEA() {
  addByte(0x5F);
}

void Z80OpcodeWriter::addLdEL() {
  addByte(0x5D);
}

void Z80OpcodeWriter::addLdCA() {
  addByte(0x4F);
}

void Z80OpcodeWriter::addLdCB() {
  addByte(0x48);
}

void Z80OpcodeWriter::addLdCD() {
  addByte(0x4A);
}

void Z80OpcodeWriter::addLdCE() {
  addByte(0x4B);
}

void Z80OpcodeWriter::addLdCH() {
  addByte(0x4C);
}

void Z80OpcodeWriter::addLdCL() {
  addByte(0x4D);
}

void Z80OpcodeWriter::addLdCiHL() {
  addByte(0x4E);
}

void Z80OpcodeWriter::addLdDA() {
  addByte(0x57);
}

void Z80OpcodeWriter::addLdDL() {
  addByte(0x55);
}

void Z80OpcodeWriter::addLdDH() {
  addByte(0x54);
}

void Z80OpcodeWriter::addLdDiHL() {
  addByte(0x56);
}

void Z80OpcodeWriter::addLdHB() {
  addByte(0x60);
}

void Z80OpcodeWriter::addLdHD() {
  addByte(0x62);
}

void Z80OpcodeWriter::addLdLC() {
  addByte(0x69);
}

void Z80OpcodeWriter::addLdLE() {
  addByte(0x6B);
}

void Z80OpcodeWriter::addLdLH() {
  addByte(0x6C);
}

void Z80OpcodeWriter::addLdHL(int n) {
  addCmd(0x21, n);
}

void Z80OpcodeWriter::addLdBC(int n) {
  addCmd(0x01, n);
}

void Z80OpcodeWriter::addLdDE() {
  addByte(0x53);
}

void Z80OpcodeWriter::addLdDE(int n) {
  addCmd(0x11, n);
}

void Z80OpcodeWriter::addLdIX(int n) {
  addByte(0xDD);
  addCmd(0x21, n);
}

void Z80OpcodeWriter::addLdAii(int i) {
  addCmd(0x3A, i);
}

void Z80OpcodeWriter::addLdHLii(int i) {
  addCmd(0x2A, i);
}

void Z80OpcodeWriter::addLdBCii(int i) {
  addByte(0xED);
  addCmd(0x4B, i);
}

void Z80OpcodeWriter::addLdDEii(int i) {
  addByte(0xED);
  addCmd(0x5B, i);
}

void Z80OpcodeWriter::addLdIXii(int i) {
  // addWord(0xDD,0xED);
  // addCmd(0x5B, i);
  addByte(0xDD);
  addLdHLii(i);
}

void Z80OpcodeWriter::addLdIYii(int i) {
  addByte(0xFD);
  addLdHLii(i);
}

void Z80OpcodeWriter::addLdiiA(int i) {
  addCmd(0x32, i);
}

void Z80OpcodeWriter::addLdiiHL(int i) {
  addCmd(0x22, i);
}

void Z80OpcodeWriter::addLdiiDE(int i) {
  addByte(0xED);
  addCmd(0x53, i);
}

void Z80OpcodeWriter::addLdiiSP(int i) {
  addByte(0xED);
  addCmd(0x73, i);
}

void Z80OpcodeWriter::addLdSPii(int i) {
  addByte(0xED);
  addCmd(0x7b, i);
}

void Z80OpcodeWriter::addLdSPHL() {
  addByte(0xF9);
}

void Z80OpcodeWriter::addLdiHL(unsigned char n) {
  addWord(0x36, n);
}

void Z80OpcodeWriter::addLdiHLA() {
  addByte(0x77);
}

void Z80OpcodeWriter::addLdiHLB() {
  addByte(0x70);
}

void Z80OpcodeWriter::addLdiHLC() {
  addByte(0x71);
}

void Z80OpcodeWriter::addLdiHLD() {
  addByte(0x72);
}

void Z80OpcodeWriter::addLdiHLE() {
  addByte(0x73);
}

void Z80OpcodeWriter::addLdiDEA() {
  addByte(0x12);
}
