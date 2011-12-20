//===-- MapipAsmPrinter.cpp - Mapip LLVM assembly writer ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to Mapip assembly language.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "mapip-asm-printer"

#include "Mapip.h"
#include "MapipMachine.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Module.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/Twine.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Target/Mangler.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
class MapipAsmPrinter : public AsmPrinter {
public:
	explicit MapipAsmPrinter(TargetMachine &TM, MCStreamer &Streamer)
		: AsmPrinter(TM, Streamer) {}

	const char *getPassName() const { return "Mapip Assembly Printer"; }

	bool doFinalization(Module &M);

	virtual void EmitStartOfAsmFile(Module &M);

	virtual void EmitFunctionBodyStart();
	virtual void EmitFunctionBodyEnd() { OutStreamer.EmitRawText(Twine("}")); }

	virtual void EmitInstruction(const MachineInstr *MI);

	void printOperand(const MachineInstr *MI, int opNum, raw_ostream &OS);
	void printMemOperand(const MachineInstr *MI, int opNum, raw_ostream &OS,
		const char *Modifier = 0);
	void printParamOperand(const MachineInstr *MI, int opNum, raw_ostream &OS,
		const char *Modifier = 0);

	// autogen'd.
	void printInstruction(const MachineInstr *MI, raw_ostream &OS);
	static const char *getRegisterName(unsigned RegNo);

private:
	void EmitVariableDeclaration(const GlobalVariable *gv);
	void EmitFunctionDeclaration();
}; // class MapipAsmPrinter
} // namespace

// Force static initialization.
extern "C" void LLVMInitializeMapipAsmPrinter() {
  RegisterAsmPrinter<MapipAsmPrinter> X(TheMapipTarget);
}


bool MapipAsmPrinter::doFinalization(Module& M) {
	return true;
}

void MapipAsmPrinter::EmitStartOfAsmFile(Module& M) {
	OutStreamer.EmitRawText(StringRef(".file\n"));
}

void MapipAsmPrinter::EmitFunctionBodyStart() {
	OutStreamer.EmitRawText(StringRef(".function\n"));
}

void MapipAsmPrinter::EmitInstruction(const MachineInstr* MI) {
	OutStreamer.EmitRawText(StringRef(".instruction\n"));
}

void MapipAsmPrinter::printOperand(const MachineInstr *MI, int opNum, raw_ostream &OS) {
	OutStreamer.EmitRawText(StringRef(".operand\n"));
}

void MapipAsmPrinter::printMemOperand(const MachineInstr *MI, int opNum, raw_ostream &OS,
	const char *Modifier)
{
	OutStreamer.EmitRawText(StringRef(".memOperand\n"));
}

void MapipAsmPrinter::printParamOperand(const MachineInstr *MI, int opNum, raw_ostream &OS,
	const char *Modifier)
{
	OutStreamer.EmitRawText(StringRef(".paramOperand\n"));
}

void MapipAsmPrinter::printInstruction(const MachineInstr *MI, raw_ostream &OS) {
	OutStreamer.EmitRawText(StringRef(".pI\n"));
}

const char* MapipAsmPrinter::getRegisterName(unsigned RegNo) {
	static const char* sRegisterNames[] = {
		"zr", "sp",  "rt",  "fr",
		"d0", "d1",  "d2",  "d3", "d4", "d5", "d6", "d7",
		"i0", "i1",  "i2",  "i3", "r0", "r1", "r2", "r3",
		"r4", "r5",  "r6",  "r7", "r8", "r9", "r10","r11",
		"r12","r13", "r14", "r15",
	};
	static const unsigned sNumReg = sizeof(sRegisterNames) / sizeof(char*);
	if(RegNo < sNumReg)
		return sRegisterNames[RegNo];
	return "Illegal register number";
}

void MapipAsmPrinter::EmitVariableDeclaration(const GlobalVariable *gv) {
	OutStreamer.EmitRawText(StringRef(".EmitVariableDeclaration\n"));
}

void MapipAsmPrinter::EmitFunctionDeclaration() {
	OutStreamer.EmitRawText(StringRef(".EmitFunctionDeclaration\n"));
}
