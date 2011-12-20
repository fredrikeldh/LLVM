//===-- MapipISelDAGToDAG.cpp - A dag to dag inst selector for Mapip ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines an instruction selector for the Mapip target.
//
//===----------------------------------------------------------------------===//

#include "MapipTargetMachine.h"
#include "llvm/Intrinsics.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

//===----------------------------------------------------------------------===//
// Instruction Selector Implementation
//===----------------------------------------------------------------------===//

//===--------------------------------------------------------------------===//
/// MapipDAGToDAGISel - Mapip specific code to select Mapip machine
/// instructions for SelectionDAG operations.
///
namespace {
class MapipDAGToDAGISel : public SelectionDAGISel {
	MapipTargetMachine& TM;
public:
	explicit MapipDAGToDAGISel(MapipTargetMachine& tm)
	: SelectionDAGISel(tm), TM(tm)
	{
	}

	SDNode *Select(SDNode *N);

	/// CheckPatternPredicate - This function is generated by tblgen in the
	/// target.  It runs the specified pattern predicate and returns true if it
	/// succeeds or false if it fails.  The number is a private implementation
	/// detail to the code tblgen produces.
	virtual bool CheckPatternPredicate(unsigned PredNo) const {
		assert(0 && "Tblgen should generate the implementation of this!");
		return 0;
	}

	/// CheckNodePredicate - This function is generated by tblgen in the target.
	/// It runs node predicate number PredNo and returns true if it succeeds or
	/// false if it fails.  The number is a private implementation
	/// detail to the code tblgen produces.
	virtual bool CheckNodePredicate(SDNode *N, unsigned PredNo) const {
		assert(0 && "Tblgen should generate the implementation of this!");
		return 0;
	}

	virtual bool CheckComplexPattern(SDNode *Root, SDNode *Parent, SDValue N,
		unsigned PatternNo, SmallVectorImpl<std::pair<SDValue, SDNode*> > &Result)
	{
		assert(0 && "Tblgen should generate the implementation of this!");
		return false;
	}

	virtual SDValue RunSDNodeXForm(SDValue V, unsigned XFormNo) {
		assert(0 && "Tblgen should generate this!");
		return SDValue();
	}

	virtual const char *getPassName() const {
		return "Mapip DAG->DAG Pattern Instruction Selection";
	}

	// Include the pieces autogenerated from the target description.
//#include "SparcGenDAGISel.inc"

private:
	SDNode* getGlobalBaseReg();
};
}  // end anonymous namespace

SDNode* SparcDAGToDAGISel::getGlobalBaseReg() {
	unsigned GlobalBaseReg = TM.getInstrInfo()->getGlobalBaseReg(MF);
	return CurDAG->getRegister(GlobalBaseReg, TLI.getPointerTy()).getNode();
}

SDNode *SparcDAGToDAGISel::Select(SDNode *N) {
	DebugLoc dl = N->getDebugLoc();
	if (N->isMachineOpcode())
		return NULL;   // Already selected.

	switch (N->getOpcode()) {
	default: break;
	case SPISD::GLOBAL_BASE_REG:
		return getGlobalBaseReg();

	case ISD::SDIV:
	case ISD::UDIV: {
		// FIXME: should use a custom expander to expose the SRA to the dag.
		SDValue DivLHS = N->getOperand(0);
		SDValue DivRHS = N->getOperand(1);

		// Set the Y register to the high-part.
		SDValue TopPart;
		if (N->getOpcode() == ISD::SDIV) {
			TopPart = SDValue(CurDAG->getMachineNode(SP::SRAri, dl, MVT::i32, DivLHS,
				CurDAG->getTargetConstant(31, MVT::i32)), 0);
		} else {
			TopPart = CurDAG->getRegister(SP::G0, MVT::i32);
		}
		TopPart = SDValue(CurDAG->getMachineNode(SP::WRYrr, dl, MVT::Glue, TopPart,
			CurDAG->getRegister(SP::G0, MVT::i32)), 0);

		// FIXME: Handle div by immediate.
		unsigned Opcode = N->getOpcode() == ISD::SDIV ? SP::SDIVrr : SP::UDIVrr;
		return CurDAG->SelectNodeTo(N, Opcode, MVT::i32, DivLHS, DivRHS, TopPart);
	}
	case ISD::MULHU:
	case ISD::MULHS: {
		// FIXME: Handle mul by immediate.
		SDValue MulLHS = N->getOperand(0);
		SDValue MulRHS = N->getOperand(1);
		unsigned Opcode = N->getOpcode() == ISD::MULHU ? SP::UMULrr : SP::SMULrr;
		SDNode *Mul = CurDAG->getMachineNode(Opcode, dl, MVT::i32, MVT::Glue,
			MulLHS, MulRHS);
		// The high part is in the Y register.
		return CurDAG->SelectNodeTo(N, SP::RDY, MVT::i32, SDValue(Mul, 1));
		return NULL;
	}
	}

	return SelectCode(N);
}

/// createSparcISelDag - This pass converts a legalized DAG into a
/// SPARC-specific DAG, ready for instruction scheduling.
///
FunctionPass *llvm::createMapipISelDag(MapipTargetMachine &TM) {
	return new MapipDAGToDAGISel(TM);
}