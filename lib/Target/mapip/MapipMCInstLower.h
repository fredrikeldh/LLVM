//===-- MapipMCInstLower.h - Lower MachineInstr to MCInst -------------------==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef MAPIPMCINSTLOWER_H
#define MAPIPMCINSTLOWER_H
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/Support/Compiler.h"

namespace llvm {
	class MCAsmInfo;
	class MCContext;
	class MCInst;
	class MCOperand;
	class MCSymbol;
	class MachineInstr;
	class MachineFunction;
	class Mangler;
	class MapipAsmPrinter;

/// MapipMCInstLower - This class is used to lower an MachineInstr into an
//                    MCInst.
class LLVM_LIBRARY_VISIBILITY MapipMCInstLower {
	typedef MachineOperand::MachineOperandType MachineOperandType;
	MCContext &Ctx;
	Mangler *Mang;
	MapipAsmPrinter &AsmPrinter;
	public:
	MapipMCInstLower(Mangler *mang, const MachineFunction &MF,
		MapipAsmPrinter &asmprinter);
	void Lower(const MachineInstr *MI, MCInst &OutMI) const;
	private:
	MCOperand LowerSymbolOperand(const MachineOperand &MO,
		MachineOperandType MOTy, unsigned Offset) const;
	MCOperand LowerOperand(const MachineOperand& MO) const;
};
}

#endif
