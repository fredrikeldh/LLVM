//===- MapipInstrInfo.cpp - Mapip Instruction Information ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Mapip implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "MapipInstrInfo.h"
#include "MapipTargetMachine.h"
#include "MapipMachineFunctionInfo.h"
#include "InstPrinter/MapipInstPrinter.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/ADT/STLExtras.h"

#define GET_INSTRINFO_CTOR
#include "MapipGenInstrInfo.inc"

using namespace llvm;

MapipInstrInfo::MapipInstrInfo(MapipTargetMachine &tm)
: MapipGenInstrInfo(),
TM(tm),
RI(*TM.getSubtargetImpl(), *this) {}


const MapipRegisterInfo &MapipInstrInfo::getRegisterInfo() const {
	return RI;
}

/// isLoadFromStackSlot - If the specified machine instruction is a direct
/// load from a stack slot, return the virtual or physical register number of
/// the destination along with the FrameIndex of the loaded stack slot.  If
/// not, return 0.  This predicate must return 0 if the instruction has
/// any side effects other than loading from the stack slot.
unsigned MapipInstrInfo::
isLoadFromStackSlot(const MachineInstr *MI, int &FrameIndex) const
{
#if 0	// POP not supported yet
	unsigned Opc = MI->getOpcode();
	if (Opc == Mapip::POP)
	{
		return 1;
	}
#endif

	return 0;
}

/// isStoreToStackSlot - If the specified machine instruction is a direct
/// store to a stack slot, return the virtual or physical register number of
/// the source reg along with the FrameIndex of the loaded stack slot.  If
/// not, return 0.  This predicate must return 0 if the instruction has
/// any side effects other than storing to the stack slot.
unsigned MapipInstrInfo::
isStoreToStackSlot(const MachineInstr *MI, int &FrameIndex) const
{
#if 0	// PUSH not supported yet
	unsigned Opc = MI->getOpcode();
	if (Opc == Mapip::PUSH)
	{
		return 1;
	}
#endif
	return 0;
}

/// insertNoop - If data hazard condition is found insert the target nop
/// instruction.
void MapipInstrInfo::
insertNoop(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI) const
{
	assert(0 && "insertNoop not implemented");
#if 0
	DebugLoc DL;
	BuildMI(MBB, MI, DL, get(Mapip::NOP));
#endif
}

void MapipInstrInfo::
copyPhysReg(MachineBasicBlock &MBB,
	MachineBasicBlock::iterator I, DebugLoc DL,
	unsigned DestReg, unsigned SrcReg,
	bool KillSrc) const
{
	unsigned Opc = 0, ZeroReg = 0;

	if (Mapip::GPRRegClass.contains(DestReg)) { // Copy to CPU Reg.
		if (Mapip::GPRRegClass.contains(SrcReg))
			Opc = Mapip::LDI, ZeroReg = Mapip::ZERO;
	}

	assert(Opc && "Cannot copy registers");

	MachineInstrBuilder MIB = BuildMI(MBB, I, DL, get(Opc));

	if (DestReg)
		MIB.addReg(DestReg, RegState::Define);

	if (ZeroReg)
		MIB.addReg(ZeroReg);

	if (SrcReg)
		MIB.addReg(SrcReg, getKillRegState(KillSrc));
}

void MapipInstrInfo::
storeRegToStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
	unsigned SrcReg, bool isKill, int FI,
	const TargetRegisterClass *RC,
	const TargetRegisterInfo *TRI) const
{
	DebugLoc DL;
	if (I != MBB.end()) DL = I->getDebugLoc();
	unsigned Opc = 0;

	if (RC == Mapip::GPRRegisterClass)
		Opc = Mapip::STW;

	assert(Opc && "Register class not handled!");
	BuildMI(MBB, I, DL, get(Opc)).addReg(SrcReg, getKillRegState(isKill))
		.addFrameIndex(FI).addImm(0);
}

void MapipInstrInfo::
loadRegFromStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
	unsigned DestReg, int FI,
	const TargetRegisterClass *RC,
	const TargetRegisterInfo *TRI) const
{
	DebugLoc DL;
	if (I != MBB.end()) DL = I->getDebugLoc();
	unsigned Opc = 0;

	if (RC == Mapip::GPRRegisterClass)
		Opc = Mapip::LDW;

	assert(Opc && "Register class not handled!");
	BuildMI(MBB, I, DL, get(Opc), DestReg).addFrameIndex(FI).addImm(0);
}

MachineInstr*
MapipInstrInfo::emitFrameIndexDebugValue(MachineFunction &MF, int FrameIx,
	uint64_t Offset, const MDNode *MDPtr,
	DebugLoc DL) const
{
	MachineInstrBuilder MIB = BuildMI(MF, DL, get(Mapip::DBG_VALUE))
		.addFrameIndex(FrameIx).addImm(0).addImm(Offset).addMetadata(MDPtr);
	return &*MIB;
}

//===----------------------------------------------------------------------===//
// Branch Analysis
//===----------------------------------------------------------------------===//

#define CONDITIONAL_JUMP_INSTRUCTIONS(m)\
	m(JC_EQ, JC_NE)\
	m(JC_NE, JC_EQ)\
	m(JC_GE, JC_LT)\
	m(JC_GEU, JC_LTU)\
	m(JC_GT, JC_LE)\
	m(JC_GTU, JC_LEU)\
	m(JC_LE, JC_GT)\
	m(JC_LEU, JC_GTU)\
	m(JC_LT, JC_GE)\
	m(JC_LTU, JC_GEU)\

#define OPC_EQUALS(opc, opposite) Opc == Mapip::opc##jc16 || Opc == Mapip::opc##jc24 ||

static unsigned GetAnalyzableBrOpc(unsigned Opc) {
	return (CONDITIONAL_JUMP_INSTRUCTIONS(OPC_EQUALS)
		Opc == Mapip::JPR || Opc == Mapip::JPI || Opc == Mapip::FAR_JPI ||
		Opc == Mapip::CALL || Opc == Mapip::CALLI || Opc == Mapip::FAR_CALLI) ?
		Opc : 0;
}

#define CASE_OPPOSITE(opc, opposite) \
case Mapip::opc##jc16: return Mapip::opposite##jc16;\
case Mapip::opc##jc24: return Mapip::opposite##jc24;\

/// GetOppositeBranchOpc - Return the inverse of the specified
/// opcode, e.g. turning BEQ to BNE.
unsigned Mapip::GetOppositeBranchOpc(unsigned Opc)
{
	switch (Opc) {
	default: llvm_unreachable("Illegal opcode!");
	CONDITIONAL_JUMP_INSTRUCTIONS(CASE_OPPOSITE)
	}
}

static void AnalyzeCondBr(const MachineInstr* Inst, unsigned Opc,
	MachineBasicBlock *&BB,
	SmallVectorImpl<MachineOperand>& Cond)
{
	assert(GetAnalyzableBrOpc(Opc) && "Not an analyzable branch");
	int NumOp = Inst->getNumExplicitOperands();

	// for both int and fp branches, the last explicit operand is the
	// MBB.
	BB = Inst->getOperand(NumOp-1).getMBB();
	Cond.push_back(MachineOperand::CreateImm(Opc));

	for (int i=0; i<NumOp-1; i++)
		Cond.push_back(Inst->getOperand(i));
}

bool MapipInstrInfo::AnalyzeBranch(MachineBasicBlock &MBB,
	MachineBasicBlock *&TBB,
	MachineBasicBlock *&FBB,
	SmallVectorImpl<MachineOperand> &Cond,
	bool AllowModify) const
{
	MachineBasicBlock::reverse_iterator I = MBB.rbegin(), REnd = MBB.rend();

	// Skip all the debug instructions.
	while (I != REnd && I->isDebugValue())
		++I;

	if (I == REnd || !isUnpredicatedTerminator(&*I)) {
		// If this block ends with no branches (it just falls through to its succ)
		// just return false, leaving TBB/FBB null.
		TBB = FBB = NULL;
		return false;
	}

	MachineInstr *LastInst = &*I;
	unsigned LastOpc = LastInst->getOpcode();

	// Not an analyzable branch (must be an indirect jump).
	if (!GetAnalyzableBrOpc(LastOpc))
		return true;

	// Get the second to last instruction in the block.
	unsigned SecondLastOpc = 0;
	MachineInstr *SecondLastInst = NULL;

	if (++I != REnd) {
		SecondLastInst = &*I;
		SecondLastOpc = GetAnalyzableBrOpc(SecondLastInst->getOpcode());

		// Not an analyzable branch (must be an indirect jump).
		if (isUnpredicatedTerminator(SecondLastInst) && !SecondLastOpc)
			return true;
	}

	// If there is only one terminator instruction, process it.
	if (!SecondLastOpc) {
		// Unconditional branch
		if (LastOpc == Mapip::JPI) {
			TBB = LastInst->getOperand(0).getMBB();
			return false;
		}

		// Conditional branch
		AnalyzeCondBr(LastInst, LastOpc, TBB, Cond);
		return false;
	}

	// If we reached here, there are two branches.
	// If there are three terminators, we don't know what sort of block this is.
	if (++I != REnd && isUnpredicatedTerminator(&*I))
		return true;

	// If second to last instruction is an unconditional branch,
	// analyze it and remove the last instruction.
	if (SecondLastOpc == Mapip::JPI) {
		// Return if the last instruction cannot be removed.
		if (!AllowModify)
			return true;

		TBB = SecondLastInst->getOperand(0).getMBB();
		LastInst->eraseFromParent();
		return false;
	}

	// Conditional branch followed by an unconditional branch.
	// The last one must be unconditional.
	if (LastOpc != Mapip::JPI)
		return true;

	AnalyzeCondBr(SecondLastInst, SecondLastOpc, TBB, Cond);
	FBB = LastInst->getOperand(0).getMBB();

	return false;
}

void MapipInstrInfo::BuildCondBr(MachineBasicBlock &MBB,
	MachineBasicBlock *TBB, DebugLoc DL,
	const SmallVectorImpl<MachineOperand>& Cond) const
{
	unsigned Opc = Cond[0].getImm();
	const MCInstrDesc &MCID = get(Opc);
	MachineInstrBuilder MIB = BuildMI(&MBB, DL, MCID);

	for (unsigned i = 1; i < Cond.size(); ++i)
		MIB.addReg(Cond[i].getReg());

	MIB.addMBB(TBB);
}

unsigned MapipInstrInfo::
InsertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
	MachineBasicBlock *FBB,
	const SmallVectorImpl<MachineOperand> &Cond,
	DebugLoc DL) const
{
	// Shouldn't be a fall through.
	assert(TBB && "InsertBranch must not be told to insert a fallthrough");

	// # of condition operands:
	//  Unconditional branches: 0
	//  Floating point branches: 1 (opc)
	//  Int BranchZero: 2 (opc, reg)
	//  Int Branch: 3 (opc, reg0, reg1)
	assert((Cond.size() <= 3) &&
				 "# of Mapip branch conditions must be <= 3!");

	// Two-way Conditional branch.
	if (FBB) {
		BuildCondBr(MBB, TBB, DL, Cond);
		BuildMI(&MBB, DL, get(Mapip::JPI)).addMBB(FBB);
		return 2;
	}

	// One way branch.
	// Unconditional branch.
	if (Cond.empty())
		BuildMI(&MBB, DL, get(Mapip::JPI)).addMBB(TBB);
	else // Conditional branch.
		BuildCondBr(MBB, TBB, DL, Cond);
	return 1;
}

unsigned MapipInstrInfo::
RemoveBranch(MachineBasicBlock &MBB) const
{
	MachineBasicBlock::reverse_iterator I = MBB.rbegin(), REnd = MBB.rend();
	MachineBasicBlock::reverse_iterator FirstBr;
	unsigned removed;

	// Skip all the debug instructions.
	while (I != REnd && I->isDebugValue())
		++I;

	FirstBr = I;

	// Up to 2 branches are removed.
	// Note that indirect branches are not removed.
	for(removed = 0; I != REnd && removed < 2; ++I, ++removed)
		if (!GetAnalyzableBrOpc(I->getOpcode()))
			break;

	MBB.erase(I.base(), FirstBr.base());

	return removed;
}

/// ReverseBranchCondition - Return the inverse opcode of the
/// specified Branch instruction.
bool MapipInstrInfo::
ReverseBranchCondition(SmallVectorImpl<MachineOperand> &Cond) const
{
	assert( (Cond.size() && Cond.size() <= 3) &&
		"Invalid Mapip branch condition!");
	Cond[0].setImm(Mapip::GetOppositeBranchOpc(Cond[0].getImm()));
	return false;
}

/// getGlobalBaseReg - Return a virtual register initialized with the
/// the global base register value. Output instructions required to
/// initialize the register in the function entry block, if necessary.
///
unsigned MapipInstrInfo::getGlobalBaseReg(MachineFunction *MF) const {
	MapipFunctionInfo *MapipFI = MF->getInfo<MapipFunctionInfo>();
	unsigned GlobalBaseReg = MapipFI->getGlobalBaseReg();
	if (GlobalBaseReg != 0)
		return GlobalBaseReg;

	assert(0 && "getGlobalBaseReg");
#if 0	// we don't support PIC yet.
	// Insert the set of GlobalBaseReg into the first MBB of the function
	MachineBasicBlock &FirstMBB = MF->front();
	MachineBasicBlock::iterator MBBI = FirstMBB.begin();
	MachineRegisterInfo &RegInfo = MF->getRegInfo();
	const TargetInstrInfo *TII = MF->getTarget().getInstrInfo();

	GlobalBaseReg = RegInfo.createVirtualRegister(Mapip::GPRRegisterClass);
	BuildMI(FirstMBB, MBBI, DebugLoc(), TII->get(TargetOpcode::COPY),
		GlobalBaseReg).addReg(Mapip::GP);
	RegInfo.addLiveIn(Mapip::GP);

	MapipFI->setGlobalBaseReg(GlobalBaseReg);
	return GlobalBaseReg;
#endif
	return 0;
}
