/***
 * @file cpu_opcode_writer.h
 * @brief CPU opcode writer abstraction for compiler application layer
 */

#ifndef CPU_OPCODE_WRITER_H
#define CPU_OPCODE_WRITER_H

#include "cpu_workspace_context.h"

class ICpuOpcodeWriter {
 public:
  ICpuOpcodeWriter();
  ICpuOpcodeWriter(CpuWorkspaceContext* context);
  virtual ~ICpuOpcodeWriter() = default;

  CpuWorkspaceContext* context;

  void addByte(unsigned char byte);
  void addWord(unsigned int word);
  void addWord(unsigned char byte1, unsigned char byte2);
  void addCmd(unsigned char byte, unsigned int word);
  void addCodeByte(unsigned char byte);
  void pushLastCode();
  void popLastCode();

  virtual void addNop() = 0;
  virtual void addExAF() = 0;
  virtual void addExDEHL() = 0;
  virtual void addExiSPHL() = 0;
  virtual void addExx() = 0;
  virtual void addEI() = 0;
  virtual void addDI() = 0;
  virtual void addPushAF() = 0;
  virtual void addPushHL() = 0;
  virtual void addPushBC() = 0;
  virtual void addPushDE() = 0;
  virtual void addPopAF() = 0;
  virtual void addPopHL() = 0;
  virtual void addPopBC() = 0;
  virtual void addPopDE() = 0;
  virtual void addPopIX() = 0;
  virtual void addPopIY() = 0;
  virtual void addXorA() = 0;
  virtual void addXorH() = 0;
  virtual void addXorE() = 0;
  virtual void addXorD() = 0;
  virtual void addAndA() = 0;
  virtual void addAndD() = 0;
  virtual void addAnd(unsigned char n) = 0;
  virtual void addOr(unsigned char n) = 0;
  virtual void addOrH() = 0;
  virtual void addOrL() = 0;
  virtual void addOrB() = 0;
  virtual void addOrC() = 0;
  virtual void addOrE() = 0;
  virtual void addCpL() = 0;
  virtual void addCpB() = 0;
  virtual void addCp(unsigned char n) = 0;
  virtual void addIncA() = 0;
  virtual void addIncH() = 0;
  virtual void addIncL() = 0;
  virtual void addIncD() = 0;
  virtual void addIncHL() = 0;
  virtual void addIncDE() = 0;
  virtual void addDecHL() = 0;
  virtual void addAdd(unsigned char n) = 0;
  virtual void addAddH() = 0;
  virtual void addAddL() = 0;
  virtual void addAddA() = 0;
  virtual void addAddHLHL() = 0;
  virtual void addAddHLBC() = 0;
  virtual void addAddHLDE() = 0;
  virtual void addAdcA(unsigned char n) = 0;
  virtual void addSub(unsigned char n) = 0;
  virtual void addSbcHLDE() = 0;
  virtual void addSbcHLBC() = 0;
  virtual void addRRCA() = 0;
  virtual void addRLA() = 0;
  virtual void addSRAH() = 0;
  virtual void addSRLH() = 0;
  virtual void addRRL() = 0;
  virtual void addRRA() = 0;
  virtual void addCPL() = 0;
  virtual void addLDI() = 0;
  virtual void addLDIR() = 0;
  virtual void addDecA() = 0;
  virtual void addDecE() = 0;
  virtual void addRet() = 0;
  virtual void addRetC() = 0;
  virtual void addLdA(unsigned char n) = 0;
  virtual void addLdAB() = 0;
  virtual void addLdAC() = 0;
  virtual void addLdAH() = 0;
  virtual void addLdAL() = 0;
  virtual void addLdAE() = 0;
  virtual void addLdAD() = 0;
  virtual void addLdAiHL() = 0;
  virtual void addLdAiDE() = 0;
  virtual void addLdHA() = 0;
  virtual void addLdLA() = 0;
  virtual void addLdLC() = 0;
  virtual void addLdLE() = 0;
  virtual void addLdLH() = 0;
  virtual void addLdL(unsigned char n) = 0;
  virtual void addLdHE() = 0;
  virtual void addLdHC() = 0;
  virtual void addLdHD() = 0;
  virtual void addLdHL() = 0;
  virtual void addLdH(unsigned char n) = 0;
  virtual void addLdLiHL() = 0;
  virtual void addLdBA() = 0;
  virtual void addLdBH() = 0;
  virtual void addLdBL() = 0;
  virtual void addLdBE() = 0;
  virtual void addLdBiHL() = 0;
  virtual void addLdB(unsigned char n) = 0;
  virtual void addLdC(unsigned char n) = 0;
  virtual void addLdEiHL() = 0;
  virtual void addLdEA() = 0;
  virtual void addLdEL() = 0;
  virtual void addLdCA() = 0;
  virtual void addLdCB() = 0;
  virtual void addLdCE() = 0;
  virtual void addLdCD() = 0;
  virtual void addLdCH() = 0;
  virtual void addLdCL() = 0;
  virtual void addLdCiHL() = 0;
  virtual void addLdDA() = 0;
  virtual void addLdDL() = 0;
  virtual void addLdDH() = 0;
  virtual void addLdDiHL() = 0;
  virtual void addLdHB() = 0;
  virtual void addLdHL(int n) = 0;
  virtual void addLdBC(int n) = 0;
  virtual void addLdDE() = 0;
  virtual void addLdDE(int n) = 0;
  virtual void addLdIX(int n) = 0;
  virtual void addLdAii(int i) = 0;
  virtual void addLdHLii(int i) = 0;
  virtual void addLdBCii(int i) = 0;
  virtual void addLdDEii(int i) = 0;
  virtual void addLdIXii(int i) = 0;
  virtual void addLdIYii(int i) = 0;
  virtual void addLdiiA(int i) = 0;
  virtual void addLdiiHL(int i) = 0;
  virtual void addLdiiDE(int i) = 0;
  virtual void addLdiiSP(int i) = 0;
  virtual void addLdSPii(int i) = 0;
  virtual void addLdSPHL() = 0;
  virtual void addLdiHL(unsigned char n) = 0;
  virtual void addLdiHLA() = 0;
  virtual void addLdiHLB() = 0;
  virtual void addLdiHLC() = 0;
  virtual void addLdiHLD() = 0;
  virtual void addLdiHLE() = 0;
  virtual void addLdiDEA() = 0;
  virtual void addJr(unsigned char n) = 0;
  virtual void addJrZ(unsigned char n) = 0;
  virtual void addJrNZ(unsigned char n) = 0;
  virtual void addJrC(unsigned char n) = 0;
  virtual void addJrNC(unsigned char n) = 0;
  virtual void addJp(int n) = 0;
  virtual void addJpZ(int n) = 0;
  virtual void addJpNZ(int n) = 0;
  virtual void addCall(unsigned int word) = 0;
};

#endif  // CPU_OPCODE_WRITER_H
