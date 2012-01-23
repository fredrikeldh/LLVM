//===-- MapipISelLowering.cpp - Mapip DAG Lowering Implementation -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that Mapip uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "mips-lower"
#include "MapipISelLowering.h"
#include "MapipMachineFunctionInfo.h"
#include "MapipTargetMachine.h"
#include "MapipTargetObjectFile.h"
#include "MapipSubtarget.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Function.h"
#include "llvm/GlobalVariable.h"
#include "llvm/Intrinsics.h"
#include "llvm/CallingConv.h"
#include "InstPrinter/MapipInstPrinter.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
using namespace llvm;

#if 0
// If I is a shifted mask, set the size (Size) and the first bit of the
// mask (Pos), and return true.
// For example, if I is 0x003ff800, (Pos, Size) = (11, 11).
static bool IsShiftedMask(uint64_t I, uint64_t &Pos, uint64_t &Size) {
	if (!isUInt<32>(I) || !isShiftedMask_32(I))
		return false;

	Size = CountPopulation_32(I);
	Pos = CountTrailingZeros_32(I);
	return true;
}
#endif

const char *MapipTargetLowering::getTargetNodeName(unsigned Opcode) const {
	switch (Opcode) {
	case MapipISD::GPRel:             return "MapipISD::GPRel";
	case MapipISD::Ret:               return "MapipISD::Ret";
	case MapipISD::DynAlloc:          return "MapipISD::DynAlloc";
	case MapipISD::Sync:              return "MapipISD::Sync";
	case MapipISD::Ext:               return "MapipISD::Ext";
	case MapipISD::Ins:               return "MapipISD::Ins";
	default:                         return NULL;
	}
}

MapipTargetLowering::MapipTargetLowering(MapipTargetMachine &TM)
	: TargetLowering(TM, new MapipTargetObjectFile()),
	Subtarget(&TM.getSubtarget<MapipSubtarget>())
{
	// Mapip does not have i1 type, so use i32 for
	// setcc operations results (slt, sgt, ...).
	setBooleanContents(ZeroOrOneBooleanContent);
	setBooleanVectorContents(ZeroOrOneBooleanContent); // FIXME: Is this correct?

	// Set up the register classes
	addRegisterClass(MVT::i32, Mapip::GPRRegisterClass);

#if 0
	// When dealing with single precision only, use libcalls
	if (!Subtarget->isSingleFloat()) {
		addRegisterClass(MVT::f64, Mapip::AFGR64RegisterClass);
	}
#endif

	// Load extented operations for i1 types must be promoted
	setLoadExtAction(ISD::EXTLOAD,  MVT::i1,  Promote);
	setLoadExtAction(ISD::ZEXTLOAD, MVT::i1,  Promote);
	setLoadExtAction(ISD::SEXTLOAD, MVT::i1,  Promote);

	// MIPS doesn't have extending float->double load/store
	setLoadExtAction(ISD::EXTLOAD, MVT::f32, Expand);
	setTruncStoreAction(MVT::f64, MVT::f32, Expand);

	// Used by legalize types to correctly generate the setcc result.
	// Without this, every float setcc comes with a AND/OR with the result,
	// we don't want this, since the fpcmp result goes to a flag register,
	// which is used implicitly by brcond and select operations.
	AddPromotedToType(ISD::SETCC, MVT::i1, MVT::i32);

	// Mapip Custom Operations
	setOperationAction(ISD::GlobalAddress,      MVT::i32,   Custom);
	setOperationAction(ISD::GlobalAddress,      MVT::i64,   Custom);
	setOperationAction(ISD::BlockAddress,       MVT::i32,   Custom);
	setOperationAction(ISD::GlobalTLSAddress,   MVT::i32,   Custom);
	setOperationAction(ISD::JumpTable,          MVT::i32,   Custom);
	setOperationAction(ISD::ConstantPool,       MVT::i32,   Custom);
	setOperationAction(ISD::SELECT,             MVT::f32,   Custom);
	setOperationAction(ISD::SELECT,             MVT::f64,   Custom);
	setOperationAction(ISD::SELECT,             MVT::i32,   Custom);
	setOperationAction(ISD::BRCOND,             MVT::Other, Custom);
	setOperationAction(ISD::DYNAMIC_STACKALLOC, MVT::i32,   Custom);
	setOperationAction(ISD::VASTART,            MVT::Other, Custom);

	setOperationAction(ISD::SDIV, MVT::i32, Expand);
	setOperationAction(ISD::SREM, MVT::i32, Expand);
	setOperationAction(ISD::UDIV, MVT::i32, Expand);
	setOperationAction(ISD::UREM, MVT::i32, Expand);
	setOperationAction(ISD::SDIV, MVT::i64, Expand);
	setOperationAction(ISD::SREM, MVT::i64, Expand);
	setOperationAction(ISD::UDIV, MVT::i64, Expand);
	setOperationAction(ISD::UREM, MVT::i64, Expand);

	// Operations not directly supported by Mapip.
	setOperationAction(ISD::BR_JT,             MVT::Other, Expand);
	setOperationAction(ISD::BR_CC,             MVT::Other, Expand);
	setOperationAction(ISD::SELECT_CC,         MVT::Other, Expand);
	setOperationAction(ISD::UINT_TO_FP,        MVT::i32,   Expand);
	setOperationAction(ISD::FP_TO_UINT,        MVT::i32,   Expand);
	setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1,    Expand);
	setOperationAction(ISD::CTPOP,             MVT::i32,   Expand);
	setOperationAction(ISD::CTTZ,              MVT::i32,   Expand);
	setOperationAction(ISD::ROTL,              MVT::i32,   Expand);
	setOperationAction(ISD::ROTL,              MVT::i64,   Expand);

	setOperationAction(ISD::SHL_PARTS,         MVT::i32,   Expand);
	setOperationAction(ISD::SRA_PARTS,         MVT::i32,   Expand);
	setOperationAction(ISD::SRL_PARTS,         MVT::i32,   Expand);
	setOperationAction(ISD::FCOPYSIGN,         MVT::f32,   Custom);
	setOperationAction(ISD::FCOPYSIGN,         MVT::f64,   Custom);
	setOperationAction(ISD::FSIN,              MVT::f32,   Expand);
	setOperationAction(ISD::FSIN,              MVT::f64,   Expand);
	setOperationAction(ISD::FCOS,              MVT::f32,   Expand);
	setOperationAction(ISD::FCOS,              MVT::f64,   Expand);
	setOperationAction(ISD::FPOWI,             MVT::f32,   Expand);
	setOperationAction(ISD::FPOW,              MVT::f32,   Expand);
	setOperationAction(ISD::FPOW,              MVT::f64,   Expand);
	setOperationAction(ISD::FLOG,              MVT::f32,   Expand);
	setOperationAction(ISD::FLOG2,             MVT::f32,   Expand);
	setOperationAction(ISD::FLOG10,            MVT::f32,   Expand);
	setOperationAction(ISD::FEXP,              MVT::f32,   Expand);
	setOperationAction(ISD::FMA,               MVT::f32,   Expand);
	setOperationAction(ISD::FMA,               MVT::f64,   Expand);

	setOperationAction(ISD::EXCEPTIONADDR,     MVT::i32, Expand);
	setOperationAction(ISD::EHSELECTION,       MVT::i32, Expand);

	setOperationAction(ISD::VAARG,             MVT::Other, Expand);
	setOperationAction(ISD::VACOPY,            MVT::Other, Expand);
	setOperationAction(ISD::VAEND,             MVT::Other, Expand);

	// Use the default for now
	setOperationAction(ISD::STACKSAVE,         MVT::Other, Expand);
	setOperationAction(ISD::STACKRESTORE,      MVT::Other, Expand);

	setOperationAction(ISD::MEMBARRIER,        MVT::Other, Custom);
	setOperationAction(ISD::ATOMIC_FENCE,      MVT::Other, Custom);

	setOperationAction(ISD::ATOMIC_LOAD,       MVT::i32,    Expand);
	setOperationAction(ISD::ATOMIC_STORE,      MVT::i32,    Expand);

	setInsertFencesForAtomic(true);

	if (Subtarget->isSingleFloat())
		setOperationAction(ISD::SELECT_CC, MVT::f64, Expand);

	if (!Subtarget->hasSEInReg()) {
		setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i8,  Expand);
		setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i16, Expand);
	}

	if (!Subtarget->hasBitCount())
		setOperationAction(ISD::CTLZ, MVT::i32, Expand);

	if (!Subtarget->hasSwap())
		setOperationAction(ISD::BSWAP, MVT::i32, Expand);

	setTargetDAGCombine(ISD::ADDE);
	setTargetDAGCombine(ISD::SUBE);
	setTargetDAGCombine(ISD::SDIVREM);
	setTargetDAGCombine(ISD::UDIVREM);
	setTargetDAGCombine(ISD::SETCC);
	setTargetDAGCombine(ISD::AND);
	setTargetDAGCombine(ISD::OR);

	setMinFunctionAlignment(1);

	setStackPointerRegisterToSaveRestore(Mapip::SP);
	computeRegisterProperties();
}

bool MapipTargetLowering::allowsUnalignedMemoryAccesses(EVT VT) const {
	MVT::SimpleValueType SVT = VT.getSimpleVT().SimpleTy;
	return SVT == MVT::i64 || SVT == MVT::i32 || SVT == MVT::i16;
}

EVT MapipTargetLowering::getSetCCResultType(EVT VT) const {
	return MVT::i32;
}

#if 0
static Mapip::CondCode FPCondCCodeToFCC(ISD::CondCode CC) {
	switch (CC) {
	default: llvm_unreachable("Unknown fp condition code!");
	case ISD::SETEQ:
	case ISD::SETOEQ: return Mapip::FCOND_OEQ;
	case ISD::SETUNE: return Mapip::FCOND_UNE;
	case ISD::SETLT:
	case ISD::SETOLT: return Mapip::FCOND_OLT;
	case ISD::SETGT:
	case ISD::SETOGT: return Mapip::FCOND_OGT;
	case ISD::SETLE:
	case ISD::SETOLE: return Mapip::FCOND_OLE;
	case ISD::SETGE:
	case ISD::SETOGE: return Mapip::FCOND_OGE;
	case ISD::SETULT: return Mapip::FCOND_ULT;
	case ISD::SETULE: return Mapip::FCOND_ULE;
	case ISD::SETUGT: return Mapip::FCOND_UGT;
	case ISD::SETUGE: return Mapip::FCOND_UGE;
	case ISD::SETUO:  return Mapip::FCOND_UN;
	case ISD::SETO:   return Mapip::FCOND_OR;
	case ISD::SETNE:
	case ISD::SETONE: return Mapip::FCOND_ONE;
	case ISD::SETUEQ: return Mapip::FCOND_UEQ;
	}
}


// Returns true if condition code has to be inverted.
static bool InvertFPCondCode(Mapip::CondCode CC) {
	if (CC >= Mapip::FCOND_F && CC <= Mapip::FCOND_NGT)
		return false;

	if (CC >= Mapip::FCOND_T && CC <= Mapip::FCOND_GT)
		return true;

	assert(false && "Illegal Condition Code");
	return false;
}

// Creates and returns an FPCmp node from a setcc node.
// Returns Op if setcc is not a floating point comparison.
static SDValue CreateFPCmp(SelectionDAG& DAG, const SDValue& Op) {
	// must be a SETCC node
	if (Op.getOpcode() != ISD::SETCC)
		return Op;

	SDValue LHS = Op.getOperand(0);

	if (!LHS.getValueType().isFloatingPoint())
		return Op;

	SDValue RHS = Op.getOperand(1);
	DebugLoc dl = Op.getDebugLoc();

	// Assume the 3rd operand is a CondCodeSDNode. Add code to check the type of
	// node if necessary.
	ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(2))->get();

	return DAG.getNode(MapipISD::FPCmp, dl, MVT::Glue, LHS, RHS,
		DAG.getConstant(FPCondCCodeToFCC(CC), MVT::i32));
}

// Creates and returns a CMovFPT/F node.
static SDValue CreateCMovFP(SelectionDAG& DAG, SDValue Cond, SDValue True,
	SDValue False, DebugLoc DL)
{
	bool invert = InvertFPCondCode((Mapip::CondCode)
		cast<ConstantSDNode>(Cond.getOperand(2))
		->getSExtValue());

	return DAG.getNode((invert ? MapipISD::CMovFP_F : MapipISD::CMovFP_T), DL,
		True.getValueType(), True, False, Cond);
}

static SDValue PerformSETCCCombine(SDNode *N, SelectionDAG& DAG,
	TargetLowering::DAGCombinerInfo &DCI,
	const MapipSubtarget* Subtarget)
{
	if (DCI.isBeforeLegalizeOps())
		return SDValue();

	SDValue Cond = CreateFPCmp(DAG, SDValue(N, 0));

	if (Cond.getOpcode() != MapipISD::FPCmp)
		return SDValue();

	SDValue True  = DAG.getConstant(1, MVT::i32);
	SDValue False = DAG.getConstant(0, MVT::i32);

	return CreateCMovFP(DAG, Cond, True, False, N->getDebugLoc());
}

static SDValue PerformANDCombine(SDNode *N, SelectionDAG& DAG,
	TargetLowering::DAGCombinerInfo &DCI,
	const MapipSubtarget* Subtarget)
{
	// Pattern match EXT.
	//  $dst = and ((sra or srl) $src , pos), (2**size - 1)
	//  => ext $dst, $src, size, pos
	if (DCI.isBeforeLegalizeOps() || !Subtarget->hasMapip32r2())
		return SDValue();

	SDValue ShiftRight = N->getOperand(0), Mask = N->getOperand(1);

	// Op's first operand must be a shift right.
	if (ShiftRight.getOpcode() != ISD::SRA && ShiftRight.getOpcode() != ISD::SRL)
		return SDValue();

	// The second operand of the shift must be an immediate.
	uint64_t Pos;
	ConstantSDNode *CN;
	if (!(CN = dyn_cast<ConstantSDNode>(ShiftRight.getOperand(1))))
		return SDValue();

	Pos = CN->getZExtValue();

	uint64_t SMPos, SMSize;
	// Op's second operand must be a shifted mask.
	if (!(CN = dyn_cast<ConstantSDNode>(Mask)) ||
		!IsShiftedMask(CN->getZExtValue(), SMPos, SMSize))
		return SDValue();

	// Return if the shifted mask does not start at bit 0 or the sum of its size
	// and Pos exceeds the word's size.
	if (SMPos != 0 || Pos + SMSize > 32)
		return SDValue();

	return DAG.getNode(MapipISD::Ext, N->getDebugLoc(), MVT::i32,
		ShiftRight.getOperand(0),
		DAG.getConstant(Pos, MVT::i32),
		DAG.getConstant(SMSize, MVT::i32));
}

static SDValue PerformORCombine(SDNode *N, SelectionDAG& DAG,
	TargetLowering::DAGCombinerInfo &DCI,
	const MapipSubtarget* Subtarget)
{
	// Pattern match INS.
	//  $dst = or (and $src1 , mask0), (and (shl $src, pos), mask1),
	//  where mask1 = (2**size - 1) << pos, mask0 = ~mask1
	//  => ins $dst, $src, size, pos, $src1
	if (DCI.isBeforeLegalizeOps() || !Subtarget->hasMapip32r2())
		return SDValue();

	SDValue And0 = N->getOperand(0), And1 = N->getOperand(1);
	uint64_t SMPos0, SMSize0, SMPos1, SMSize1;
	ConstantSDNode *CN;

	// See if Op's first operand matches (and $src1 , mask0).
	if (And0.getOpcode() != ISD::AND)
		return SDValue();

	if (!(CN = dyn_cast<ConstantSDNode>(And0.getOperand(1))) ||
		!IsShiftedMask(~CN->getSExtValue(), SMPos0, SMSize0))
		return SDValue();

	// See if Op's second operand matches (and (shl $src, pos), mask1).
	if (And1.getOpcode() != ISD::AND)
		return SDValue();

	if (!(CN = dyn_cast<ConstantSDNode>(And1.getOperand(1))) ||
		!IsShiftedMask(CN->getZExtValue(), SMPos1, SMSize1))
		return SDValue();

	// The shift masks must have the same position and size.
	if (SMPos0 != SMPos1 || SMSize0 != SMSize1)
		return SDValue();

	SDValue Shl = And1.getOperand(0);
	if (Shl.getOpcode() != ISD::SHL)
		return SDValue();

	if (!(CN = dyn_cast<ConstantSDNode>(Shl.getOperand(1))))
		return SDValue();

	unsigned Shamt = CN->getZExtValue();

	// Return if the shift amount and the first bit position of mask are not the
	// same.
	if (Shamt != SMPos0)
		return SDValue();

	return DAG.getNode(MapipISD::Ins, N->getDebugLoc(), MVT::i32,
		Shl.getOperand(0),
		DAG.getConstant(SMPos0, MVT::i32),
		DAG.getConstant(SMSize0, MVT::i32),
		And0.getOperand(0));
}
#endif

SDValue  MapipTargetLowering::PerformDAGCombine(SDNode *N, DAGCombinerInfo &DCI)
	const
{
#if 0
	SelectionDAG &DAG = DCI.DAG;
	unsigned opc = N->getOpcode();

	switch (opc) {
	default: break;
	case ISD::AND:
		return PerformANDCombine(N, DAG, DCI, Subtarget);
	case ISD::OR:
		return PerformORCombine(N, DAG, DCI, Subtarget);
	}
#endif

	return SDValue();
}

SDValue MapipTargetLowering::
	LowerOperation(SDValue Op, SelectionDAG &DAG) const
{
	switch (Op.getOpcode())
	{
	case ISD::BRCOND:             return LowerBRCOND(Op, DAG);
	case ISD::ConstantPool:       return LowerConstantPool(Op, DAG);
	case ISD::DYNAMIC_STACKALLOC: return LowerDYNAMIC_STACKALLOC(Op, DAG);
	//case ISD::GlobalAddress:      return LowerGlobalAddress(Op, DAG);
	//case ISD::BlockAddress:       return LowerBlockAddress(Op, DAG);
	//case ISD::GlobalTLSAddress:   return LowerGlobalTLSAddress(Op, DAG);
	case ISD::JumpTable:          return LowerJumpTable(Op, DAG);
	case ISD::SELECT:             return LowerSELECT(Op, DAG);
	case ISD::VASTART:            return LowerVASTART(Op, DAG);
	//case ISD::FCOPYSIGN:          return LowerFCOPYSIGN(Op, DAG);
	//case ISD::FRAMEADDR:          return LowerFRAMEADDR(Op, DAG);
	//case ISD::MEMBARRIER:         return LowerMEMBARRIER(Op, DAG);
	//case ISD::ATOMIC_FENCE:       return LowerATOMIC_FENCE(Op, DAG);
	}
	return SDValue();
}

//===----------------------------------------------------------------------===//
//  Lower helper functions
//===----------------------------------------------------------------------===//

// AddLiveIn - This helper function adds the specified physical register to the
// MachineFunction as a live in value.  It also creates a corresponding
// virtual register for it.
static unsigned
	AddLiveIn(MachineFunction &MF, unsigned PReg, TargetRegisterClass *RC)
{
	assert(RC->contains(PReg) && "Not the correct regclass!");
	unsigned VReg = MF.getRegInfo().createVirtualRegister(RC);
	MF.getRegInfo().addLiveIn(PReg, VReg);
	return VReg;
}

#if 0
// Get fp branch code (not opcode) from condition code.
static Mapip::FPBranchCode GetFPBranchCodeFromCond(Mapip::CondCode CC) {
	if (CC >= Mapip::FCOND_F && CC <= Mapip::FCOND_NGT)
		return Mapip::BRANCH_T;

	if (CC >= Mapip::FCOND_T && CC <= Mapip::FCOND_GT)
		return Mapip::BRANCH_F;

	return Mapip::BRANCH_INVALID;
}

static MachineBasicBlock* ExpandCondMov(MachineInstr *MI, MachineBasicBlock *BB,
	DebugLoc dl,
	const MapipSubtarget* Subtarget,
	const TargetInstrInfo *TII,
	bool isFPCmp, unsigned Opc)
{
	// There is no need to expand CMov instructions if target has
	// conditional moves.
	if (Subtarget->hasCondMov())
		return BB;

	// To "insert" a SELECT_CC instruction, we actually have to insert the
	// diamond control-flow pattern.  The incoming instruction knows the
	// destination vreg to set, the condition code register to branch on, the
	// true/false values to select between, and a branch opcode to use.
	const BasicBlock *LLVM_BB = BB->getBasicBlock();
	MachineFunction::iterator It = BB;
	++It;

	//  thisMBB:
	//  ...
	//   TrueVal = ...
	//   setcc r1, r2, r3
	//   bNE   r1, r0, copy1MBB
	//   fallthrough --> copy0MBB
	MachineBasicBlock *thisMBB  = BB;
	MachineFunction *F = BB->getParent();
	MachineBasicBlock *copy0MBB = F->CreateMachineBasicBlock(LLVM_BB);
	MachineBasicBlock *sinkMBB  = F->CreateMachineBasicBlock(LLVM_BB);
	F->insert(It, copy0MBB);
	F->insert(It, sinkMBB);

	// Transfer the remainder of BB and its successor edges to sinkMBB.
	sinkMBB->splice(sinkMBB->begin(), BB,
		llvm::next(MachineBasicBlock::iterator(MI)),
		BB->end());
	sinkMBB->transferSuccessorsAndUpdatePHIs(BB);

	// Next, add the true and fallthrough blocks as its successors.
	BB->addSuccessor(copy0MBB);
	BB->addSuccessor(sinkMBB);

	// Emit the right instruction according to the type of the operands compared
	if (isFPCmp)
		BuildMI(BB, dl, TII->get(Opc)).addMBB(sinkMBB);
	else
		BuildMI(BB, dl, TII->get(Opc)).addReg(MI->getOperand(2).getReg())
		.addReg(Mapip::ZERO).addMBB(sinkMBB);

	//  copy0MBB:
	//   %FalseValue = ...
	//   # fallthrough to sinkMBB
	BB = copy0MBB;

	// Update machine-CFG edges
	BB->addSuccessor(sinkMBB);

	//  sinkMBB:
	//   %Result = phi [ %TrueValue, thisMBB ], [ %FalseValue, copy0MBB ]
	//  ...
	BB = sinkMBB;

	if (isFPCmp)
		BuildMI(*BB, BB->begin(), dl,
		TII->get(Mapip::PHI), MI->getOperand(0).getReg())
		.addReg(MI->getOperand(2).getReg()).addMBB(thisMBB)
		.addReg(MI->getOperand(1).getReg()).addMBB(copy0MBB);
	else
		BuildMI(*BB, BB->begin(), dl,
		TII->get(Mapip::PHI), MI->getOperand(0).getReg())
		.addReg(MI->getOperand(3).getReg()).addMBB(thisMBB)
		.addReg(MI->getOperand(1).getReg()).addMBB(copy0MBB);

	MI->eraseFromParent();   // The pseudo instruction is gone now.
	return BB;
}
#endif

//===----------------------------------------------------------------------===//
//  Misc Lower Operation implementation
//===----------------------------------------------------------------------===//
SDValue MapipTargetLowering::
	LowerDYNAMIC_STACKALLOC(SDValue Op, SelectionDAG &DAG) const
{
	MachineFunction &MF = DAG.getMachineFunction();
	MapipFunctionInfo *MapipFI = MF.getInfo<MapipFunctionInfo>();

	assert(getTargetMachine().getFrameLowering()->getStackAlignment() >=
		cast<ConstantSDNode>(Op.getOperand(2).getNode())->getZExtValue() &&
		"Cannot lower if the alignment of the allocated space is larger than \
		that of the stack.");

	SDValue Chain = Op.getOperand(0);
	SDValue Size = Op.getOperand(1);
	DebugLoc dl = Op.getDebugLoc();

	// Get a reference from Mapip stack pointer
	SDValue StackPointer = DAG.getCopyFromReg(Chain, dl, Mapip::SP, MVT::i32);

	// Subtract the dynamic size from the actual stack size to
	// obtain the new stack size.
	SDValue Sub = DAG.getNode(ISD::SUB, dl, MVT::i32, StackPointer, Size);

	// The Sub result contains the new stack start address, so it
	// must be placed in the stack pointer register.
	Chain = DAG.getCopyToReg(StackPointer.getValue(1), dl, Mapip::SP, Sub,
		SDValue());

	// This node always has two return values: a new stack pointer
	// value and a chain
	SDVTList VTLs = DAG.getVTList(MVT::i32, MVT::Other);
	SDValue Ptr = DAG.getFrameIndex(MapipFI->getDynAllocFI(), getPointerTy());
	SDValue Ops[] = { Chain, Ptr, Chain.getValue(1) };

	return DAG.getNode(MapipISD::DynAlloc, dl, VTLs, Ops, 3);
}

#if 0
SDValue MapipTargetLowering::
	LowerBRCOND(SDValue Op, SelectionDAG &DAG) const
{
	// The first operand is the chain, the second is the condition, the third is
	// the block to branch to if the condition is true.
	SDValue Chain = Op.getOperand(0);
	SDValue Dest = Op.getOperand(2);
	DebugLoc dl = Op.getDebugLoc();

	SDValue CondRes = CreateFPCmp(DAG, Op.getOperand(1));

	// Return if flag is not set by a floating point comparison.
	if (CondRes.getOpcode() != MapipISD::FPCmp)
		return Op;

	SDValue CCNode  = CondRes.getOperand(2);
	Mapip::CondCode CC =
		(Mapip::CondCode)cast<ConstantSDNode>(CCNode)->getZExtValue();
	SDValue BrCode = DAG.getConstant(GetFPBranchCodeFromCond(CC), MVT::i32);

	return DAG.getNode(MapipISD::FPBrcond, dl, Op.getValueType(), Chain, BrCode,
		Dest, CondRes);
}

SDValue MapipTargetLowering::
	LowerSELECT(SDValue Op, SelectionDAG &DAG) const
{
	SDValue Cond = CreateFPCmp(DAG, Op.getOperand(0));

	// Return if flag is not set by a floating point comparison.
	if (Cond.getOpcode() != MapipISD::FPCmp)
		return Op;

	return CreateCMovFP(DAG, Cond, Op.getOperand(1), Op.getOperand(2),
		Op.getDebugLoc());
}

SDValue MapipTargetLowering::LowerGlobalAddress(SDValue Op,
	SelectionDAG &DAG) const
{
	// FIXME there isn't actually debug info here
	DebugLoc dl = Op.getDebugLoc();
	const GlobalValue *GV = cast<GlobalAddressSDNode>(Op)->getGlobal();

	if (getTargetMachine().getRelocationModel() != Reloc::PIC_) {
		SDVTList VTs = DAG.getVTList(MVT::i32);

		MapipTargetObjectFile &TLOF = (MapipTargetObjectFile&)getObjFileLowering();

		// %gp_rel relocation
		if (TLOF.IsGlobalInSmallSection(GV, getTargetMachine())) {
			SDValue GA = DAG.getTargetGlobalAddress(GV, dl, MVT::i32, 0,
				MapipII::MO_GPREL);
			SDValue GPRelNode = DAG.getNode(MapipISD::GPRel, dl, VTs, &GA, 1);
			SDValue GOT = DAG.getGLOBAL_OFFSET_TABLE(MVT::i32);
			return DAG.getNode(ISD::ADD, dl, MVT::i32, GOT, GPRelNode);
		}
		// %hi/%lo relocation
		SDValue GAHi = DAG.getTargetGlobalAddress(GV, dl, MVT::i32, 0,
			MapipII::MO_ABS_HI);
		SDValue GALo = DAG.getTargetGlobalAddress(GV, dl, MVT::i32, 0,
			MapipII::MO_ABS_LO);
		SDValue HiPart = DAG.getNode(MapipISD::Hi, dl, VTs, &GAHi, 1);
		SDValue Lo = DAG.getNode(MapipISD::Lo, dl, MVT::i32, GALo);
		return DAG.getNode(ISD::ADD, dl, MVT::i32, HiPart, Lo);
	}

	EVT ValTy = Op.getValueType();
	bool HasGotOfst = (GV->hasInternalLinkage() ||
		(GV->hasLocalLinkage() && !isa<Function>(GV)));
	unsigned GotFlag = MapipII::MO_GOT;
	SDValue GA = DAG.getTargetGlobalAddress(GV, dl, ValTy, 0, GotFlag);
	GA = DAG.getNode(MapipISD::WrapperPIC, dl, ValTy, GA);
	SDValue ResNode = DAG.getLoad(ValTy, dl,
		DAG.getEntryNode(), GA, MachinePointerInfo(),
		false, false, 0);
	// On functions and global targets not internal linked only
	// a load from got/GP is necessary for PIC to work.
	if (!HasGotOfst)
		return ResNode;
	SDValue GALo = DAG.getTargetGlobalAddress(GV, dl, ValTy, 0,
		MapipII::MO_ABS_LO);
	SDValue Lo = DAG.getNode(MapipISD::Lo, dl, ValTy, GALo);
	return DAG.getNode(ISD::ADD, dl, ValTy, ResNode, Lo);
}

SDValue MapipTargetLowering::LowerBlockAddress(SDValue Op,
	SelectionDAG &DAG) const
{
	const BlockAddress *BA = cast<BlockAddressSDNode>(Op)->getBlockAddress();
	// FIXME there isn't actually debug info here
	DebugLoc dl = Op.getDebugLoc();

	if (getTargetMachine().getRelocationModel() != Reloc::PIC_) {
		// %hi/%lo relocation
		SDValue BAHi = DAG.getBlockAddress(BA, MVT::i32, true,
			MapipII::MO_ABS_HI);
		SDValue BALo = DAG.getBlockAddress(BA, MVT::i32, true,
			MapipII::MO_ABS_LO);
		SDValue Hi = DAG.getNode(MapipISD::Hi, dl, MVT::i32, BAHi);
		SDValue Lo = DAG.getNode(MapipISD::Lo, dl, MVT::i32, BALo);
		return DAG.getNode(ISD::ADD, dl, MVT::i32, Hi, Lo);
	}

	SDValue BAGOTOffset = DAG.getBlockAddress(BA, MVT::i32, true,
		MapipII::MO_GOT);
	BAGOTOffset = DAG.getNode(MapipISD::WrapperPIC, dl, MVT::i32, BAGOTOffset);
	SDValue BALOOffset = DAG.getBlockAddress(BA, MVT::i32, true,
		MapipII::MO_ABS_LO);
	SDValue Load = DAG.getLoad(MVT::i32, dl,
		DAG.getEntryNode(), BAGOTOffset,
		MachinePointerInfo(), false, false, 0);
	SDValue Lo = DAG.getNode(MapipISD::Lo, dl, MVT::i32, BALOOffset);
	return DAG.getNode(ISD::ADD, dl, MVT::i32, Load, Lo);
}
#endif

SDValue MapipTargetLowering::
	LowerJumpTable(SDValue Op, SelectionDAG &DAG) const
{
	SDValue ResNode;
	SDValue HiPart;
	// FIXME there isn't actually debug info here
	DebugLoc dl = Op.getDebugLoc();
	bool IsPIC = getTargetMachine().getRelocationModel() == Reloc::PIC_;
	unsigned char OpFlag = IsPIC ? MapipII::MO_GOT : MapipII::MO_ABS_HI;

	EVT PtrVT = Op.getValueType();
	JumpTableSDNode *JT  = cast<JumpTableSDNode>(Op);

	SDValue JTI = DAG.getTargetJumpTable(JT->getIndex(), PtrVT, OpFlag);

	if (!IsPIC) {
		SDValue Ops[] = { JTI };
		HiPart = DAG.getNode(MapipISD::Hi, dl, DAG.getVTList(MVT::i32), Ops, 1);
	} else {// Emit Load from Global Pointer
		JTI = DAG.getNode(MapipISD::WrapperPIC, dl, MVT::i32, JTI);
		HiPart = DAG.getLoad(MVT::i32, dl, DAG.getEntryNode(), JTI,
			MachinePointerInfo(),
			false, false, 0);
	}

	SDValue JTILo = DAG.getTargetJumpTable(JT->getIndex(), PtrVT,
		MapipII::MO_ABS_LO);
	SDValue Lo = DAG.getNode(MapipISD::Lo, dl, MVT::i32, JTILo);
	ResNode = DAG.getNode(ISD::ADD, dl, MVT::i32, HiPart, Lo);

	return ResNode;
}

SDValue MapipTargetLowering::
	LowerConstantPool(SDValue Op, SelectionDAG &DAG) const
{
	SDValue ResNode;
	ConstantPoolSDNode *N = cast<ConstantPoolSDNode>(Op);
	const Constant *C = N->getConstVal();
	// FIXME there isn't actually debug info here
	DebugLoc dl = Op.getDebugLoc();

	// gp_rel relocation
	// FIXME: we should reference the constant pool using small data sections,
	// but the asm printer currently doesn't support this feature without
	// hacking it. This feature should come soon so we can uncomment the
	// stuff below.
	//if (IsInSmallSection(C->getType())) {
	//  SDValue GPRelNode = DAG.getNode(MapipISD::GPRel, MVT::i32, CP);
	//  SDValue GOT = DAG.getGLOBAL_OFFSET_TABLE(MVT::i32);
	//  ResNode = DAG.getNode(ISD::ADD, MVT::i32, GOT, GPRelNode);

	if (getTargetMachine().getRelocationModel() != Reloc::PIC_) {
		SDValue CPHi = DAG.getTargetConstantPool(C, MVT::i32, N->getAlignment(),
			N->getOffset(), MapipII::MO_ABS_HI);
		SDValue CPLo = DAG.getTargetConstantPool(C, MVT::i32, N->getAlignment(),
			N->getOffset(), MapipII::MO_ABS_LO);
		SDValue HiPart = DAG.getNode(MapipISD::Hi, dl, MVT::i32, CPHi);
		SDValue Lo = DAG.getNode(MapipISD::Lo, dl, MVT::i32, CPLo);
		ResNode = DAG.getNode(ISD::ADD, dl, MVT::i32, HiPart, Lo);
	} else {
		SDValue CP = DAG.getTargetConstantPool(C, MVT::i32, N->getAlignment(),
			N->getOffset(), MapipII::MO_GOT);
		CP = DAG.getNode(MapipISD::WrapperPIC, dl, MVT::i32, CP);
		SDValue Load = DAG.getLoad(MVT::i32, dl, DAG.getEntryNode(),
			CP, MachinePointerInfo::getConstantPool(),
			false, false, 0);
		SDValue CPLo = DAG.getTargetConstantPool(C, MVT::i32, N->getAlignment(),
			N->getOffset(), MapipII::MO_ABS_LO);
		SDValue Lo = DAG.getNode(MapipISD::Lo, dl, MVT::i32, CPLo);
		ResNode = DAG.getNode(ISD::ADD, dl, MVT::i32, Load, Lo);
	}

	return ResNode;
}

SDValue MapipTargetLowering::LowerVASTART(SDValue Op, SelectionDAG &DAG) const {
	MachineFunction &MF = DAG.getMachineFunction();
	MapipFunctionInfo *FuncInfo = MF.getInfo<MapipFunctionInfo>();

	DebugLoc dl = Op.getDebugLoc();
	SDValue FI = DAG.getFrameIndex(FuncInfo->getVarArgsFrameIndex(),
		getPointerTy());

	// vastart just stores the address of the VarArgsFrameIndex slot into the
	// memory location argument.
	const Value *SV = cast<SrcValueSDNode>(Op.getOperand(2))->getValue();
	return DAG.getStore(Op.getOperand(0), dl, FI, Op.getOperand(1),
		MachinePointerInfo(SV),
		false, false, 0);
}

static SDValue LowerFCOPYSIGN32(SDValue Op, SelectionDAG &DAG) {
	// FIXME: Use ext/ins instructions if target architecture is Mapip32r2.
	DebugLoc dl = Op.getDebugLoc();
	SDValue Op0 = DAG.getNode(ISD::BITCAST, dl, MVT::i32, Op.getOperand(0));
	SDValue Op1 = DAG.getNode(ISD::BITCAST, dl, MVT::i32, Op.getOperand(1));
	SDValue And0 = DAG.getNode(ISD::AND, dl, MVT::i32, Op0,
		DAG.getConstant(0x7fffffff, MVT::i32));
	SDValue And1 = DAG.getNode(ISD::AND, dl, MVT::i32, Op1,
		DAG.getConstant(0x80000000, MVT::i32));
	SDValue Result = DAG.getNode(ISD::OR, dl, MVT::i32, And0, And1);
	return DAG.getNode(ISD::BITCAST, dl, MVT::f32, Result);
}

static SDValue LowerFCOPYSIGN64(SDValue Op, SelectionDAG &DAG, bool isLittle) {
	// FIXME:
	//  Use ext/ins instructions if target architecture is Mapip32r2.
	//  Eliminate redundant mfc1 and mtc1 instructions.
	unsigned LoIdx = 0, HiIdx = 1;

	if (!isLittle)
		std::swap(LoIdx, HiIdx);

	DebugLoc dl = Op.getDebugLoc();
	SDValue Word0 = DAG.getNode(MapipISD::ExtractElementF64, dl, MVT::i32,
		Op.getOperand(0),
		DAG.getConstant(LoIdx, MVT::i32));
	SDValue Hi0 = DAG.getNode(MapipISD::ExtractElementF64, dl, MVT::i32,
		Op.getOperand(0), DAG.getConstant(HiIdx, MVT::i32));
	SDValue Hi1 = DAG.getNode(MapipISD::ExtractElementF64, dl, MVT::i32,
		Op.getOperand(1), DAG.getConstant(HiIdx, MVT::i32));
	SDValue And0 = DAG.getNode(ISD::AND, dl, MVT::i32, Hi0,
		DAG.getConstant(0x7fffffff, MVT::i32));
	SDValue And1 = DAG.getNode(ISD::AND, dl, MVT::i32, Hi1,
		DAG.getConstant(0x80000000, MVT::i32));
	SDValue Word1 = DAG.getNode(ISD::OR, dl, MVT::i32, And0, And1);

	if (!isLittle)
		std::swap(Word0, Word1);

	return DAG.getNode(MapipISD::BuildPairF64, dl, MVT::f64, Word0, Word1);
}

SDValue MapipTargetLowering::LowerFCOPYSIGN(SDValue Op, SelectionDAG &DAG)
	const
{
	EVT Ty = Op.getValueType();

	assert(Ty == MVT::f32 || Ty == MVT::f64);

	if (Ty == MVT::f32)
		return LowerFCOPYSIGN32(Op, DAG);
	else
		return LowerFCOPYSIGN64(Op, DAG, Subtarget->isLittle());
}

SDValue MapipTargetLowering::
	LowerFRAMEADDR(SDValue Op, SelectionDAG &DAG) const
{
	// check the depth
	assert((cast<ConstantSDNode>(Op.getOperand(0))->getZExtValue() == 0) &&
		"Frame address can only be determined for current frame.");

	MachineFrameInfo *MFI = DAG.getMachineFunction().getFrameInfo();
	MFI->setFrameAddressIsTaken(true);
	EVT VT = Op.getValueType();
	DebugLoc dl = Op.getDebugLoc();
	SDValue FrameAddr = DAG.getCopyFromReg(DAG.getEntryNode(), dl, Mapip::FP, VT);
	return FrameAddr;
}

// TODO: set SType according to the desired memory barrier behavior.
SDValue MapipTargetLowering::LowerMEMBARRIER(SDValue Op,
	SelectionDAG& DAG) const
{
	unsigned SType = 0;
	DebugLoc dl = Op.getDebugLoc();
	return DAG.getNode(MapipISD::Sync, dl, MVT::Other, Op.getOperand(0),
		DAG.getConstant(SType, MVT::i32));
}

SDValue MapipTargetLowering::LowerATOMIC_FENCE(SDValue Op,
	SelectionDAG& DAG) const
{
	// FIXME: Need pseudo-fence for 'singlethread' fences
	// FIXME: Set SType for weaker fences where supported/appropriate.
	unsigned SType = 0;
	DebugLoc dl = Op.getDebugLoc();
	return DAG.getNode(MapipISD::Sync, dl, MVT::Other, Op.getOperand(0),
		DAG.getConstant(SType, MVT::i32));
}

//===----------------------------------------------------------------------===//
//                      Calling Convention Implementation
//===----------------------------------------------------------------------===//

#include "MapipGenCallingConv.inc"

//===----------------------------------------------------------------------===//
// TODO: Implement a generic logic using tblgen that can support this.
// Mapip O32 ABI rules:
// ---
// i32 - Passed in P0, P1, P2, P3 and stack
// f64 - todo
//
//  For vararg functions, all arguments are passed in A0, A1, A2, A3 and stack.
//===----------------------------------------------------------------------===//

static bool CC_MapipO32(unsigned ValNo, MVT ValVT,
	MVT LocVT, CCValAssign::LocInfo LocInfo,
	ISD::ArgFlagsTy ArgFlags, CCState &State)
{

	static const unsigned IntRegsSize=4;//, FloatRegsSize=2;

	static const unsigned IntRegs[] = {
		Mapip::P0, Mapip::P1, Mapip::P2, Mapip::P3
	};
#if 0
	static const unsigned F64Regs[] = {
		Mapip::D6, Mapip::D7
	};
#endif

	// ByVal Args
	if (ArgFlags.isByVal()) {
		State.HandleByVal(ValNo, ValVT, LocVT, LocInfo,
			1 /*MinSize*/, 4 /*MinAlign*/, ArgFlags);
		unsigned NextReg = (State.getNextStackOffset() + 3) / 4;
		for (unsigned r = State.getFirstUnallocated(IntRegs, IntRegsSize);
			r < std::min(IntRegsSize, NextReg); ++r)
			State.AllocateReg(IntRegs[r]);
		return false;
	}

	// Promote i8 and i16
	if (LocVT == MVT::i8 || LocVT == MVT::i16) {
		LocVT = MVT::i32;
		if (ArgFlags.isSExt())
			LocInfo = CCValAssign::SExt;
		else if (ArgFlags.isZExt())
			LocInfo = CCValAssign::ZExt;
		else
			LocInfo = CCValAssign::AExt;
	}

	unsigned Reg;

	// f32 and f64 are allocated in A0, A1, A2, A3 when either of the following
	// is true: function is vararg, argument is 3rd or higher, there is previous
	// argument which is not f32 or f64.
#if 0
	bool AllocateFloatsInIntReg = State.isVarArg() || ValNo > 1
		|| State.getFirstUnallocated(F32Regs, FloatRegsSize) != ValNo;
#endif
	unsigned OrigAlign = ArgFlags.getOrigAlign();
	bool isI64 = (ValVT == MVT::i32 && OrigAlign == 8);

	if (ValVT == MVT::i32) {
		Reg = State.AllocateReg(IntRegs, IntRegsSize);
		// If this is the first part of an i64 arg,
		// the allocated register must be either A0 or A2.
		if (isI64 && (Reg == Mapip::P1 || Reg == Mapip::P3))
			Reg = State.AllocateReg(IntRegs, IntRegsSize);
		LocVT = MVT::i32;
#if 0
	} else if (ValVT == MVT::f64 && AllocateFloatsInIntReg) {
		// Allocate int register and shadow next int register. If first
		// available register is Mapip::A1 or Mapip::A3, shadow it too.
		Reg = State.AllocateReg(IntRegs, IntRegsSize);
		if (Reg == Mapip::A1 || Reg == Mapip::A3)
			Reg = State.AllocateReg(IntRegs, IntRegsSize);
		State.AllocateReg(IntRegs, IntRegsSize);
		LocVT = MVT::i32;
	} else if (ValVT.isFloatingPoint() && !AllocateFloatsInIntReg) {
		// we are guaranteed to find an available float register
		if (ValVT == MVT::f32) {
			Reg = State.AllocateReg(F32Regs, FloatRegsSize);
			// Shadow int register
			State.AllocateReg(IntRegs, IntRegsSize);
		} else {
			Reg = State.AllocateReg(F64Regs, FloatRegsSize);
			// Shadow int registers
			unsigned Reg2 = State.AllocateReg(IntRegs, IntRegsSize);
			if (Reg2 == Mapip::A1 || Reg2 == Mapip::A3)
				State.AllocateReg(IntRegs, IntRegsSize);
			State.AllocateReg(IntRegs, IntRegsSize);
		}
#endif
	} else
		llvm_unreachable("Cannot handle this ValVT.");

	unsigned SizeInBytes = ValVT.getSizeInBits() >> 3;
	unsigned Offset = State.AllocateStack(SizeInBytes, OrigAlign);

	if (!Reg)
		State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset, LocVT, LocInfo));
	else
		State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));

	return false; // CC must always match
}

//===----------------------------------------------------------------------===//
//                  Call Calling Convention Implementation
//===----------------------------------------------------------------------===//

static const unsigned O32IntRegsSize = 4;

static const unsigned O32IntRegs[] = {
	Mapip::P0, Mapip::P1, Mapip::P2, Mapip::P3
};

// Return next O32 integer argument register.
static unsigned getNextIntArgReg(unsigned Reg) {
	assert((Reg == Mapip::P0) || (Reg == Mapip::P2));
	return (Reg == Mapip::P0) ? Mapip::P1 : Mapip::P3;
}

// Write ByVal Arg to arg registers and stack.
static void
	WriteByValArg(SDValue& ByValChain, SDValue Chain, DebugLoc dl,
	SmallVector<std::pair<unsigned, SDValue>, 16>& RegsToPass,
	SmallVector<SDValue, 8>& MemOpChains, int& LastFI,
	MachineFrameInfo *MFI, SelectionDAG &DAG, SDValue Arg,
	const CCValAssign &VA, const ISD::ArgFlagsTy& Flags,
	MVT PtrType, bool isLittle)
{
	unsigned LocMemOffset = VA.getLocMemOffset();
	unsigned Offset = 0;
	uint32_t RemainingSize = Flags.getByValSize();
	unsigned ByValAlign = Flags.getByValAlign();

	// Copy the first 4 words of byval arg to registers A0 - A3.
	// FIXME: Use a stricter alignment if it enables better optimization in passes
	//        run later.
	for (; RemainingSize >= 4 && LocMemOffset < 4 * 4;
		Offset += 4, RemainingSize -= 4, LocMemOffset += 4) {
			SDValue LoadPtr = DAG.getNode(ISD::ADD, dl, MVT::i32, Arg,
				DAG.getConstant(Offset, MVT::i32));
			SDValue LoadVal = DAG.getLoad(MVT::i32, dl, Chain, LoadPtr,
				MachinePointerInfo(),
				false, false, std::min(ByValAlign,
				(unsigned )4));
			MemOpChains.push_back(LoadVal.getValue(1));
			unsigned DstReg = O32IntRegs[LocMemOffset / 4];
			RegsToPass.push_back(std::make_pair(DstReg, LoadVal));
	}

	if (RemainingSize == 0)
		return;

	// If there still is a register available for argument passing, write the
	// remaining part of the structure to it using subword loads and shifts.
	if (LocMemOffset < 4 * 4) {
		assert(RemainingSize <= 3 && RemainingSize >= 1 &&
			"There must be one to three bytes remaining.");
		unsigned LoadSize = (RemainingSize == 3 ? 2 : RemainingSize);
		SDValue LoadPtr = DAG.getNode(ISD::ADD, dl, MVT::i32, Arg,
			DAG.getConstant(Offset, MVT::i32));
		unsigned Alignment = std::min(ByValAlign, (unsigned )4);
		SDValue LoadVal = DAG.getExtLoad(ISD::ZEXTLOAD, dl, MVT::i32, Chain,
			LoadPtr, MachinePointerInfo(),
			MVT::getIntegerVT(LoadSize * 8), false,
			false, Alignment);
		MemOpChains.push_back(LoadVal.getValue(1));

		// If target is big endian, shift it to the most significant half-word or
		// byte.
		if (!isLittle)
			LoadVal = DAG.getNode(ISD::SHL, dl, MVT::i32, LoadVal,
			DAG.getConstant(32 - LoadSize * 8, MVT::i32));

		Offset += LoadSize;
		RemainingSize -= LoadSize;

		// Read second subword if necessary.
		if (RemainingSize != 0)  {
			assert(RemainingSize == 1 && "There must be one byte remaining.");
			LoadPtr = DAG.getNode(ISD::ADD, dl, MVT::i32, Arg,
				DAG.getConstant(Offset, MVT::i32));
			unsigned Alignment = std::min(ByValAlign, (unsigned )2);
			SDValue Subword = DAG.getExtLoad(ISD::ZEXTLOAD, dl, MVT::i32, Chain,
				LoadPtr, MachinePointerInfo(),
				MVT::i8, false, false, Alignment);
			MemOpChains.push_back(Subword.getValue(1));
			// Insert the loaded byte to LoadVal.
			// FIXME: Use INS if supported by target.
			unsigned ShiftAmt = isLittle ? 16 : 8;
			SDValue Shift = DAG.getNode(ISD::SHL, dl, MVT::i32, Subword,
				DAG.getConstant(ShiftAmt, MVT::i32));
			LoadVal = DAG.getNode(ISD::OR, dl, MVT::i32, LoadVal, Shift);
		}

		unsigned DstReg = O32IntRegs[LocMemOffset / 4];
		RegsToPass.push_back(std::make_pair(DstReg, LoadVal));
		return;
	}

	// Create a fixed object on stack at offset LocMemOffset and copy
	// remaining part of byval arg to it using memcpy.
	SDValue Src = DAG.getNode(ISD::ADD, dl, MVT::i32, Arg,
		DAG.getConstant(Offset, MVT::i32));
	LastFI = MFI->CreateFixedObject(RemainingSize, LocMemOffset, true);
	SDValue Dst = DAG.getFrameIndex(LastFI, PtrType);
	ByValChain = DAG.getMemcpy(ByValChain, dl, Dst, Src,
		DAG.getConstant(RemainingSize, MVT::i32),
		std::min(ByValAlign, (unsigned)4),
		/*isVolatile=*/false, /*AlwaysInline=*/false,
		MachinePointerInfo(0), MachinePointerInfo(0));
}

/// LowerCall - functions arguments are copied from virtual regs to
/// (physical regs)/(stack frame), CALLSEQ_START and CALLSEQ_END are emitted.
/// TODO: isTailCall.
SDValue
	MapipTargetLowering::LowerCall(SDValue InChain, SDValue Callee,
	CallingConv::ID CallConv, bool isVarArg,
	bool &isTailCall,
	const SmallVectorImpl<ISD::OutputArg> &Outs,
	const SmallVectorImpl<SDValue> &OutVals,
	const SmallVectorImpl<ISD::InputArg> &Ins,
	DebugLoc dl, SelectionDAG &DAG,
	SmallVectorImpl<SDValue> &InVals) const
{
	// MIPs target does not yet support tail call optimization.
	isTailCall = false;

	MachineFunction &MF = DAG.getMachineFunction();
	MachineFrameInfo *MFI = MF.getFrameInfo();
	const TargetFrameLowering *TFL = MF.getTarget().getFrameLowering();
	bool IsPIC = getTargetMachine().getRelocationModel() == Reloc::PIC_;
	MapipFunctionInfo *MapipFI = MF.getInfo<MapipFunctionInfo>();

	// Analyze operands of the call, assigning locations to each operand.
	SmallVector<CCValAssign, 16> ArgLocs;
	CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
		getTargetMachine(), ArgLocs, *DAG.getContext());

	CCInfo.AnalyzeCallOperands(Outs, CC_MapipO32);

	// Get a count of how many bytes are to be pushed on the stack.
	unsigned NextStackOffset = CCInfo.getNextStackOffset();

	// Chain is the output chain of the last Load/Store or CopyToReg node.
	// ByValChain is the output chain of the last Memcpy node created for copying
	// byval arguments to the stack.
	SDValue Chain, CallSeqStart, ByValChain;
	SDValue NextStackOffsetVal = DAG.getIntPtrConstant(NextStackOffset, true);
	Chain = CallSeqStart = DAG.getCALLSEQ_START(InChain, NextStackOffsetVal);
	ByValChain = InChain;

	// If this is the first call, create a stack frame object that points to
	// a location to which .cprestore saves $gp.
	if (IsPIC && !MapipFI->getGPFI())
		MapipFI->setGPFI(MFI->CreateFixedObject(4, 0, true));

	// Get the frame index of the stack frame object that points to the location
	// of dynamically allocated area on the stack.
	int DynAllocFI = MapipFI->getDynAllocFI();

	// Update size of the maximum argument space.
	// For O32, a minimum of four words (16 bytes) of argument space is
	// allocated.
	NextStackOffset = std::max(NextStackOffset, (unsigned)16);

	unsigned MaxCallFrameSize = MapipFI->getMaxCallFrameSize();

	if (MaxCallFrameSize < NextStackOffset) {
		MapipFI->setMaxCallFrameSize(NextStackOffset);

		// Set the offsets relative to $sp of the $gp restore slot and dynamically
		// allocated stack space. These offsets must be aligned to a boundary
		// determined by the stack alignment of the ABI.
		unsigned StackAlignment = TFL->getStackAlignment();
		NextStackOffset = (NextStackOffset + StackAlignment - 1) /
			StackAlignment * StackAlignment;

		if (IsPIC)
			MFI->setObjectOffset(MapipFI->getGPFI(), NextStackOffset);

		MFI->setObjectOffset(DynAllocFI, NextStackOffset);
	}

	// With EABI is it possible to have 16 args on registers.
	SmallVector<std::pair<unsigned, SDValue>, 16> RegsToPass;
	SmallVector<SDValue, 8> MemOpChains;

	int FirstFI = -MFI->getNumFixedObjects() - 1, LastFI = 0;

	// Walk the register/memloc assignments, inserting copies/loads.
	for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
		SDValue Arg = OutVals[i];
		CCValAssign &VA = ArgLocs[i];

		// Promote the value if needed.
		switch (VA.getLocInfo()) {
		default: llvm_unreachable("Unknown loc info!");
		case CCValAssign::Full:
			if (VA.isRegLoc()) {
				if (VA.getValVT() == MVT::f32 && VA.getLocVT() == MVT::i32)
					Arg = DAG.getNode(ISD::BITCAST, dl, MVT::i32, Arg);
				if (VA.getValVT() == MVT::f64 && VA.getLocVT() == MVT::i32) {
					SDValue Lo = DAG.getNode(MapipISD::ExtractElementF64, dl, MVT::i32,
						Arg, DAG.getConstant(0, MVT::i32));
					SDValue Hi = DAG.getNode(MapipISD::ExtractElementF64, dl, MVT::i32,
						Arg, DAG.getConstant(1, MVT::i32));
					if (!Subtarget->isLittle())
						std::swap(Lo, Hi);
					unsigned LocRegLo = VA.getLocReg();
					unsigned LocRegHigh = getNextIntArgReg(LocRegLo);
					RegsToPass.push_back(std::make_pair(LocRegLo, Lo));
					RegsToPass.push_back(std::make_pair(LocRegHigh, Hi));
					continue;
				}
			}
			break;
		case CCValAssign::SExt:
			Arg = DAG.getNode(ISD::SIGN_EXTEND, dl, VA.getLocVT(), Arg);
			break;
		case CCValAssign::ZExt:
			Arg = DAG.getNode(ISD::ZERO_EXTEND, dl, VA.getLocVT(), Arg);
			break;
		case CCValAssign::AExt:
			Arg = DAG.getNode(ISD::ANY_EXTEND, dl, VA.getLocVT(), Arg);
			break;
		}

		// Arguments that can be passed on register must be kept at
		// RegsToPass vector
		if (VA.isRegLoc()) {
			RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
			continue;
		}

		// Register can't get to this point...
		assert(VA.isMemLoc());

		// ByVal Arg.
		ISD::ArgFlagsTy Flags = Outs[i].Flags;
		if (Flags.isByVal()) {
			assert(Subtarget->isABI_O32() &&
				"No support for ByVal args by ABIs other than O32 yet.");
			assert(Flags.getByValSize() &&
				"ByVal args of size 0 should have been ignored by front-end.");
			WriteByValArg(ByValChain, Chain, dl, RegsToPass, MemOpChains, LastFI, MFI,
				DAG, Arg, VA, Flags, getPointerTy(), Subtarget->isLittle());
			continue;
		}

		// Create the frame index object for this incoming parameter
		LastFI = MFI->CreateFixedObject(VA.getValVT().getSizeInBits()/8,
			VA.getLocMemOffset(), true);
		SDValue PtrOff = DAG.getFrameIndex(LastFI, getPointerTy());

		// emit ISD::STORE whichs stores the
		// parameter value to a stack Location
		MemOpChains.push_back(DAG.getStore(Chain, dl, Arg, PtrOff,
			MachinePointerInfo(),
			false, false, 0));
	}

	// Extend range of indices of frame objects for outgoing arguments that were
	// created during this function call. Skip this step if no such objects were
	// created.
	if (LastFI)
		MapipFI->extendOutArgFIRange(FirstFI, LastFI);

	// If a memcpy has been created to copy a byval arg to a stack, replace the
	// chain input of CallSeqStart with ByValChain.
	if (InChain != ByValChain)
		DAG.UpdateNodeOperands(CallSeqStart.getNode(), ByValChain,
		NextStackOffsetVal);

	// Transform all store nodes into one single node because all store
	// nodes are independent of each other.
	if (!MemOpChains.empty())
		Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other,
		&MemOpChains[0], MemOpChains.size());

	// If the callee is a GlobalAddress/ExternalSymbol node (quite common, every
	// direct call is) turn it into a TargetGlobalAddress/TargetExternalSymbol
	// node so that legalize doesn't hack it.
	unsigned char OpFlag = IsPIC ? MapipII::MO_GOT_CALL : MapipII::MO_NO_FLAG;
	//bool LoadSymAddr = false;
	SDValue CalleeLo;

	if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee)) {
		if (IsPIC && G->getGlobal()->hasInternalLinkage()) {
			Callee = DAG.getTargetGlobalAddress(G->getGlobal(), dl,
				getPointerTy(), 0,MapipII:: MO_GOT);
			CalleeLo = DAG.getTargetGlobalAddress(G->getGlobal(), dl, getPointerTy(),
				0, MapipII::MO_ABS_LO);
		} else {
			Callee = DAG.getTargetGlobalAddress(G->getGlobal(), dl,
				getPointerTy(), 0, OpFlag);
		}

		//LoadSymAddr = true;
	}
	else if (ExternalSymbolSDNode *S = dyn_cast<ExternalSymbolSDNode>(Callee)) {
		Callee = DAG.getTargetExternalSymbol(S->getSymbol(),
			getPointerTy(), OpFlag);
		//LoadSymAddr = true;
	}

	SDValue InFlag;

#if 0
	// Create nodes that load address of callee and copy it to T9
	if (IsPIC) {
		if (LoadSymAddr) {
			// Load callee address
			Callee = DAG.getNode(MapipISD::WrapperPIC, dl, MVT::i32, Callee);
			SDValue LoadValue = DAG.getLoad(MVT::i32, dl, DAG.getEntryNode(), Callee,
				MachinePointerInfo::getGOT(),
				false, false, 0);

			// Use GOT+LO if callee has internal linkage.
			if (CalleeLo.getNode()) {
				SDValue Lo = DAG.getNode(MapipISD::Lo, dl, MVT::i32, CalleeLo);
				Callee = DAG.getNode(ISD::ADD, dl, MVT::i32, LoadValue, Lo);
			} else
				Callee = LoadValue;
		}

		// copy to T9
		Chain = DAG.getCopyToReg(Chain, dl, Mapip::T9, Callee, SDValue(0, 0));
		InFlag = Chain.getValue(1);
		Callee = DAG.getRegister(Mapip::T9, MVT::i32);
	}
#endif

	// Build a sequence of copy-to-reg nodes chained together with token
	// chain and flag operands which copy the outgoing args into registers.
	// The InFlag in necessary since all emitted instructions must be
	// stuck together.
	for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i) {
		Chain = DAG.getCopyToReg(Chain, dl, RegsToPass[i].first,
			RegsToPass[i].second, InFlag);
		InFlag = Chain.getValue(1);
	}

	// MapipJmpLink = #chain, #target_address, #opt_in_flags...
	//             = Chain, Callee, Reg#1, Reg#2, ...
	//
	// Returns a chain & a flag for retval copy to use.
	SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
	SmallVector<SDValue, 8> Ops;
	Ops.push_back(Chain);
	Ops.push_back(Callee);

	// Add argument registers to the end of the list so that they are
	// known live into the call.
	for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i)
		Ops.push_back(DAG.getRegister(RegsToPass[i].first,
		RegsToPass[i].second.getValueType()));

	if (InFlag.getNode())
		Ops.push_back(InFlag);

	Chain  = DAG.getNode(MapipISD::JmpLink, dl, NodeTys, &Ops[0], Ops.size());
	InFlag = Chain.getValue(1);

	// Create the CALLSEQ_END node.
	Chain = DAG.getCALLSEQ_END(Chain,
		DAG.getIntPtrConstant(NextStackOffset, true),
		DAG.getIntPtrConstant(0, true), InFlag);
	InFlag = Chain.getValue(1);

	// Handle result values, copying them out of physregs into vregs that we
	// return.
	return LowerCallResult(Chain, InFlag, CallConv, isVarArg,
		Ins, dl, DAG, InVals);
}

#if 0	// we don't have RetCC
/// LowerCallResult - Lower the result values of a call into the
/// appropriate copies out of appropriate physical registers.
SDValue
	MapipTargetLowering::LowerCallResult(SDValue Chain, SDValue InFlag,
	CallingConv::ID CallConv, bool isVarArg,
	const SmallVectorImpl<ISD::InputArg> &Ins,
	DebugLoc dl, SelectionDAG &DAG,
	SmallVectorImpl<SDValue> &InVals) const
{
	// Assign locations to each value returned by this call.
	SmallVector<CCValAssign, 16> RVLocs;
	CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
		getTargetMachine(), RVLocs, *DAG.getContext());

	CCInfo.AnalyzeCallResult(Ins, RetCC_Mapip);

	// Copy all of the result registers out of their specified physreg.
	for (unsigned i = 0; i != RVLocs.size(); ++i) {
		Chain = DAG.getCopyFromReg(Chain, dl, RVLocs[i].getLocReg(),
			RVLocs[i].getValVT(), InFlag).getValue(1);
		InFlag = Chain.getValue(2);
		InVals.push_back(Chain.getValue(0));
	}

	return Chain;
}
#endif

//===----------------------------------------------------------------------===//
//             Formal Arguments Calling Convention Implementation
//===----------------------------------------------------------------------===//
static void ReadByValArg(MachineFunction &MF, SDValue Chain, DebugLoc dl,
	std::vector<SDValue>& OutChains,
	SelectionDAG &DAG, unsigned NumWords, SDValue FIN,
	const CCValAssign &VA, const ISD::ArgFlagsTy& Flags)
{
	unsigned LocMem = VA.getLocMemOffset();
	unsigned FirstWord = LocMem / 4;

	// copy register A0 - A3 to frame object
	for (unsigned i = 0; i < NumWords; ++i) {
		unsigned CurWord = FirstWord + i;
		if (CurWord >= O32IntRegsSize)
			break;

		unsigned SrcReg = O32IntRegs[CurWord];
		unsigned Reg = AddLiveIn(MF, SrcReg, Mapip::GPRRegisterClass);
		SDValue StorePtr = DAG.getNode(ISD::ADD, dl, MVT::i32, FIN,
			DAG.getConstant(i * 4, MVT::i32));
		SDValue Store = DAG.getStore(Chain, dl, DAG.getRegister(Reg, MVT::i32),
			StorePtr, MachinePointerInfo(), false,
			false, 0);
		OutChains.push_back(Store);
	}
}

/// LowerFormalArguments - transform physical registers into virtual registers
/// and generate load operations for arguments places on the stack.
SDValue
	MapipTargetLowering::LowerFormalArguments(SDValue Chain,
	CallingConv::ID CallConv,
	bool isVarArg,
	const SmallVectorImpl<ISD::InputArg>
	&Ins,
	DebugLoc dl, SelectionDAG &DAG,
	SmallVectorImpl<SDValue> &InVals)
	const
{
	MachineFunction &MF = DAG.getMachineFunction();
	MachineFrameInfo *MFI = MF.getFrameInfo();
	MapipFunctionInfo *MapipFI = MF.getInfo<MapipFunctionInfo>();

	MapipFI->setVarArgsFrameIndex(0);

	// Used with vargs to acumulate store chains.
	std::vector<SDValue> OutChains;

	// Assign locations to all of the incoming arguments.
	SmallVector<CCValAssign, 16> ArgLocs;
	CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
		getTargetMachine(), ArgLocs, *DAG.getContext());

	CCInfo.AnalyzeFormalArguments(Ins, CC_MapipO32);

	int LastFI = 0;// MapipFI->LastInArgFI is 0 at the entry of this function.

	for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
		CCValAssign &VA = ArgLocs[i];

		// Arguments stored on registers
		if (VA.isRegLoc()) {
			EVT RegVT = VA.getLocVT();
			unsigned ArgReg = VA.getLocReg();
			TargetRegisterClass *RC = 0;

			if (RegVT == MVT::i32)
				RC = Mapip::GPRRegisterClass;
#if 0
			else if (RegVT == MVT::i64)
				RC = Mapip::CPU64RegsRegisterClass;
			else if (RegVT == MVT::f32)
				RC = Mapip::FGR32RegisterClass;
			else if (RegVT == MVT::f64)
				RC = HasMapip64 ? Mapip::FGR64RegisterClass : Mapip::AFGR64RegisterClass;
#endif
			else
				llvm_unreachable("RegVT not supported by FormalArguments Lowering");

			// Transform the arguments stored on
			// physical registers into virtual ones
			unsigned Reg = AddLiveIn(DAG.getMachineFunction(), ArgReg, RC);
			SDValue ArgValue = DAG.getCopyFromReg(Chain, dl, Reg, RegVT);

			// If this is an 8 or 16-bit value, it has been passed promoted
			// to 32 bits.  Insert an assert[sz]ext to capture this, then
			// truncate to the right size.
			if (VA.getLocInfo() != CCValAssign::Full) {
				unsigned Opcode = 0;
				if (VA.getLocInfo() == CCValAssign::SExt)
					Opcode = ISD::AssertSext;
				else if (VA.getLocInfo() == CCValAssign::ZExt)
					Opcode = ISD::AssertZext;
				if (Opcode)
					ArgValue = DAG.getNode(Opcode, dl, RegVT, ArgValue,
					DAG.getValueType(VA.getValVT()));
				ArgValue = DAG.getNode(ISD::TRUNCATE, dl, VA.getValVT(), ArgValue);
			}

			// Handle O32 ABI cases: i32->f32 and (i32,i32)->f64
			//if (Subtarget->isABI_O32())
			{
				if (RegVT == MVT::i32 && VA.getValVT() == MVT::f32)
					ArgValue = DAG.getNode(ISD::BITCAST, dl, MVT::f32, ArgValue);
				if (RegVT == MVT::i32 && VA.getValVT() == MVT::f64) {
					unsigned Reg2 = AddLiveIn(DAG.getMachineFunction(),
						getNextIntArgReg(ArgReg), RC);
					SDValue ArgValue2 = DAG.getCopyFromReg(Chain, dl, Reg2, RegVT);
					if (!Subtarget->isLittle())
						std::swap(ArgValue, ArgValue2);
					ArgValue = DAG.getNode(MapipISD::BuildPairF64, dl, MVT::f64,
						ArgValue, ArgValue2);
				}
			}

			InVals.push_back(ArgValue);
		} else { // VA.isRegLoc()

			// sanity check
			assert(VA.isMemLoc());

			ISD::ArgFlagsTy Flags = Ins[i].Flags;

			if (Flags.isByVal()) {
				assert(Subtarget->isABI_O32() &&
					"No support for ByVal args by ABIs other than O32 yet.");
				assert(Flags.getByValSize() &&
					"ByVal args of size 0 should have been ignored by front-end.");
				unsigned NumWords = (Flags.getByValSize() + 3) / 4;
				LastFI = MFI->CreateFixedObject(NumWords * 4, VA.getLocMemOffset(),
					true);
				SDValue FIN = DAG.getFrameIndex(LastFI, getPointerTy());
				InVals.push_back(FIN);
				ReadByValArg(MF, Chain, dl, OutChains, DAG, NumWords, FIN, VA, Flags);

				continue;
			}

			// The stack pointer offset is relative to the caller stack frame.
			LastFI = MFI->CreateFixedObject(VA.getValVT().getSizeInBits()/8,
				VA.getLocMemOffset(), true);

			// Create load nodes to retrieve arguments from the stack
			SDValue FIN = DAG.getFrameIndex(LastFI, getPointerTy());
			InVals.push_back(DAG.getLoad(VA.getValVT(), dl, Chain, FIN,
				MachinePointerInfo::getFixedStack(LastFI),
				false, false, 0));
		}
	}

	// The mips ABIs for returning structs by value requires that we copy
	// the sret argument into $v0 for the return. Save the argument into
	// a virtual register so that we can access it from the return points.
	if (DAG.getMachineFunction().getFunction()->hasStructRetAttr()) {
		unsigned Reg = MapipFI->getSRetReturnReg();
		if (!Reg) {
			Reg = MF.getRegInfo().createVirtualRegister(getRegClassFor(MVT::i32));
			MapipFI->setSRetReturnReg(Reg);
		}
		SDValue Copy = DAG.getCopyToReg(DAG.getEntryNode(), dl, Reg, InVals[0]);
		Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other, Copy, Chain);
	}

	if (isVarArg) {
		// Record the frame index of the first variable argument
		// which is a value necessary to VASTART.
		unsigned NextStackOffset = CCInfo.getNextStackOffset();
		assert(NextStackOffset % 4 == 0 &&
			"NextStackOffset must be aligned to 4-byte boundaries.");
		LastFI = MFI->CreateFixedObject(4, NextStackOffset, true);
		MapipFI->setVarArgsFrameIndex(LastFI);

		// If NextStackOffset is smaller than o32's 16-byte reserved argument area,
		// copy the integer registers that have not been used for argument passing
		// to the caller's stack frame.
		for (; NextStackOffset < 16; NextStackOffset += 4) {
			TargetRegisterClass *RC = Mapip::GPRRegisterClass;
			unsigned Idx = NextStackOffset / 4;
			unsigned Reg = AddLiveIn(DAG.getMachineFunction(), O32IntRegs[Idx], RC);
			SDValue ArgValue = DAG.getCopyFromReg(Chain, dl, Reg, MVT::i32);
			LastFI = MFI->CreateFixedObject(4, NextStackOffset, true);
			SDValue PtrOff = DAG.getFrameIndex(LastFI, getPointerTy());
			OutChains.push_back(DAG.getStore(Chain, dl, ArgValue, PtrOff,
				MachinePointerInfo(),
				false, false, 0));
		}
	}

	MapipFI->setLastInArgFI(LastFI);

	// All stores are grouped in one node to allow the matching between
	// the size of Ins and InVals. This only happens when on varg functions
	if (!OutChains.empty()) {
		OutChains.push_back(Chain);
		Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other,
			&OutChains[0], OutChains.size());
	}

	return Chain;
}

#if 0
//===----------------------------------------------------------------------===//
//               Return Value Calling Convention Implementation
//===----------------------------------------------------------------------===//

SDValue
	MapipTargetLowering::LowerReturn(SDValue Chain,
	CallingConv::ID CallConv, bool isVarArg,
	const SmallVectorImpl<ISD::OutputArg> &Outs,
	const SmallVectorImpl<SDValue> &OutVals,
	DebugLoc dl, SelectionDAG &DAG) const
{

	// CCValAssign - represent the assignment of
	// the return value to a location
	SmallVector<CCValAssign, 16> RVLocs;

	// CCState - Info about the registers and stack slot.
	CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
		getTargetMachine(), RVLocs, *DAG.getContext());

	// Analize return values.
	CCInfo.AnalyzeReturn(Outs, RetCC_Mapip);

	// If this is the first return lowered for this function, add
	// the regs to the liveout set for the function.
	if (DAG.getMachineFunction().getRegInfo().liveout_empty()) {
		for (unsigned i = 0; i != RVLocs.size(); ++i)
			if (RVLocs[i].isRegLoc())
				DAG.getMachineFunction().getRegInfo().addLiveOut(RVLocs[i].getLocReg());
	}

	SDValue Flag;

	// Copy the result values into the output registers.
	for (unsigned i = 0; i != RVLocs.size(); ++i) {
		CCValAssign &VA = RVLocs[i];
		assert(VA.isRegLoc() && "Can only return in registers!");

		Chain = DAG.getCopyToReg(Chain, dl, VA.getLocReg(),
			OutVals[i], Flag);

		// guarantee that all emitted copies are
		// stuck together, avoiding something bad
		Flag = Chain.getValue(1);
	}

	// The mips ABIs for returning structs by value requires that we copy
	// the sret argument into $v0 for the return. We saved the argument into
	// a virtual register in the entry block, so now we copy the value out
	// and into $v0.
	if (DAG.getMachineFunction().getFunction()->hasStructRetAttr()) {
		MachineFunction &MF      = DAG.getMachineFunction();
		MapipFunctionInfo *MapipFI = MF.getInfo<MapipFunctionInfo>();
		unsigned Reg = MapipFI->getSRetReturnReg();

		if (!Reg)
			llvm_unreachable("sret virtual register not created in the entry block");
		SDValue Val = DAG.getCopyFromReg(Chain, dl, Reg, getPointerTy());

		Chain = DAG.getCopyToReg(Chain, dl, Mapip::V0, Val, Flag);
		Flag = Chain.getValue(1);
	}

	// Return on Mapip is always a "jr $ra"
	if (Flag.getNode())
		return DAG.getNode(MapipISD::Ret, dl, MVT::Other,
		Chain, DAG.getRegister(Mapip::RA, MVT::i32), Flag);
	else // Return Void
		return DAG.getNode(MapipISD::Ret, dl, MVT::Other,
		Chain, DAG.getRegister(Mapip::RA, MVT::i32));
}
#endif

//===----------------------------------------------------------------------===//
//                           Mapip Inline Assembly Support
//===----------------------------------------------------------------------===//

/// getConstraintType - Given a constraint letter, return the type of
/// constraint it is for this target.
MapipTargetLowering::ConstraintType MapipTargetLowering::
	getConstraintType(const std::string &Constraint) const
{
	// Mapip specific constrainy
	// GCC config/mips/constraints.md
	//
	// 'd' : An address register. Equivalent to r
	//       unless generating MIPS16 code.
	// 'y' : Equivalent to r; retained for
	//       backwards compatibility.
	// 'f' : Floating Point registers.
	if (Constraint.size() == 1) {
		switch (Constraint[0]) {
		default : break;
		case 'd':
		case 'y':
		case 'f':
			return C_RegisterClass;
			break;
		}
	}
	return TargetLowering::getConstraintType(Constraint);
}

/// Examine constraint type and operand type and determine a weight value.
/// This object must already have been set up with the operand type
/// and the current alternative constraint selected.
TargetLowering::ConstraintWeight
	MapipTargetLowering::getSingleConstraintMatchWeight(
	AsmOperandInfo &info, const char *constraint) const
{
	ConstraintWeight weight = CW_Invalid;
	Value *CallOperandVal = info.CallOperandVal;
	// If we don't have a value, we can't do a match,
	// but allow it at the lowest weight.
	if (CallOperandVal == NULL)
		return CW_Default;
	Type *type = CallOperandVal->getType();
	// Look at the constraint type.
	switch (*constraint) {
	default:
		weight = TargetLowering::getSingleConstraintMatchWeight(info, constraint);
		break;
	case 'd':
	case 'y':
		if (type->isIntegerTy())
			weight = CW_Register;
		break;
	case 'f':
		if (type->isFloatTy())
			weight = CW_Register;
		break;
	}
	return weight;
}

/// Given a register class constraint, like 'r', if this corresponds directly
/// to an LLVM register class, return a register of 0 and the register class
/// pointer.
std::pair<unsigned, const TargetRegisterClass*> MapipTargetLowering::
	getRegForInlineAsmConstraint(const std::string &Constraint, EVT VT) const
{
	if (Constraint.size() == 1) {
		switch (Constraint[0]) {
		case 'd': // Address register. Same as 'r' unless generating MIPS16 code.
		case 'y': // Same as 'r'. Exists for compatibility.
		case 'r':
			return std::make_pair(0U, Mapip::GPRRegisterClass);
#if 0
		case 'f':
			if (VT == MVT::f64)
				if ((!Subtarget->isSingleFloat()) && (!Subtarget->isFP64bit()))
					return std::make_pair(0U, Mapip::AFGR64RegisterClass);
			break;
#endif
		}
	}
	return TargetLowering::getRegForInlineAsmConstraint(Constraint, VT);
}

bool
	MapipTargetLowering::isOffsetFoldingLegal(const GlobalAddressSDNode *GA) const
{
	// The Mapip target isn't yet aware of offsets.
	return false;
}

bool MapipTargetLowering::isFPImmLegal(const APFloat &Imm, EVT VT) const {
	if (VT != MVT::f32 && VT != MVT::f64)
		return false;
	if (Imm.isNegZero())
		return false;
	return Imm.isZero();
}
