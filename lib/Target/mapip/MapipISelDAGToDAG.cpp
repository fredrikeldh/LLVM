//===-- MapipISelDAGToDAG.cpp - A dag to dag inst selector for Mapip ----------===//
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

#include "Mapip.h"
#include "MapipTargetMachine.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
// MapipDAGToDAGISel - Mapip specific code to select Mapip machine
// instructions for SelectionDAG operations.
class MapipDAGToDAGISel : public SelectionDAGISel {
  public:
    MapipDAGToDAGISel(MapipTargetMachine &TM, CodeGenOpt::Level OptLevel);

    virtual const char *getPassName() const {
      return "Mapip DAG->DAG Pattern Instruction Selection";
    }

    SDNode *Select(SDNode *Node);

    // Complex Pattern Selectors.
    bool SelectADDRrr(SDValue &Addr, SDValue &R1, SDValue &R2);
    bool SelectADDRri(SDValue &Addr, SDValue &Base, SDValue &Offset);
    bool SelectADDRii(SDValue &Addr, SDValue &Base, SDValue &Offset);

    // Include the pieces auto'gened from the target description
#include "MapipGenDAGISel.inc"

  private:
    SDNode *SelectREAD_PARAM(SDNode *Node);

    bool isImm(const SDValue &operand);
    bool SelectImm(const SDValue &operand, SDValue &imm);

    const MapipSubtarget& getSubtarget() const;
}; // class MapipDAGToDAGISel
} // namespace

// createMapipISelDag - This pass converts a legalized DAG into a
// Mapip-specific DAG, ready for instruction scheduling
FunctionPass *llvm::createMapipISelDag(MapipTargetMachine &TM,
                                     CodeGenOpt::Level OptLevel) {
  return new MapipDAGToDAGISel(TM, OptLevel);
}

MapipDAGToDAGISel::MapipDAGToDAGISel(MapipTargetMachine &TM,
                                 CodeGenOpt::Level OptLevel)
  : SelectionDAGISel(TM, OptLevel) {}

SDNode *MapipDAGToDAGISel::Select(SDNode *Node) {
  if (Node->getOpcode() == MapipISD::READ_PARAM)
    return SelectREAD_PARAM(Node);
  else
    return SelectCode(Node);
}

SDNode *MapipDAGToDAGISel::SelectREAD_PARAM(SDNode *Node) {
  SDValue  index = Node->getOperand(1);
  DebugLoc dl    = Node->getDebugLoc();
  unsigned opcode;

  if (index.getOpcode() != ISD::TargetConstant)
    llvm_unreachable("READ_PARAM: index is not ISD::TargetConstant");

  if (Node->getValueType(0) == MVT::i16) {
    opcode = Mapip::LDpiU16;
  }
  else if (Node->getValueType(0) == MVT::i32) {
    opcode = Mapip::LDpiU32;
  }
  else if (Node->getValueType(0) == MVT::i64) {
    opcode = Mapip::LDpiU64;
  }
  else if (Node->getValueType(0) == MVT::f32) {
    opcode = Mapip::LDpiF32;
  }
  else if (Node->getValueType(0) == MVT::f64) {
    opcode = Mapip::LDpiF64;
  }
  else {
    llvm_unreachable("Unknown parameter type for ld.param");
  }

  return MapipInstrInfo::
    GetMapipMachineNode(CurDAG, opcode, dl, Node->getValueType(0), index);
}

// Match memory operand of the form [reg+reg]
bool MapipDAGToDAGISel::SelectADDRrr(SDValue &Addr, SDValue &R1, SDValue &R2) {
  if (Addr.getOpcode() != ISD::ADD || Addr.getNumOperands() < 2 ||
      isImm(Addr.getOperand(0)) || isImm(Addr.getOperand(1)))
    return false;

  R1 = Addr;
  R2 = CurDAG->getTargetConstant(0, MVT::i32);
  return true;
}

// Match memory operand of the form [reg], [imm+reg], and [reg+imm]
bool MapipDAGToDAGISel::SelectADDRri(SDValue &Addr, SDValue &Base,
                                   SDValue &Offset) {
  if (Addr.getOpcode() != ISD::ADD) {
    // let SelectADDRii handle the [imm] case
    if (isImm(Addr))
      return false;
    // it is [reg]
    Base = Addr;
    Offset = CurDAG->getTargetConstant(0, MVT::i32);
    return true;
  }

  if (Addr.getNumOperands() < 2)
    return false;

  // let SelectADDRii handle the [imm+imm] case
  if (isImm(Addr.getOperand(0)) && isImm(Addr.getOperand(1)))
    return false;

  // try [reg+imm] and [imm+reg]
  for (int i = 0; i < 2; i ++)
    if (SelectImm(Addr.getOperand(1-i), Offset)) {
      Base = Addr.getOperand(i);
      return true;
    }

  // neither [reg+imm] nor [imm+reg]
  return false;
}

// Match memory operand of the form [imm+imm] and [imm]
bool MapipDAGToDAGISel::SelectADDRii(SDValue &Addr, SDValue &Base,
                                   SDValue &Offset) {
  // is [imm+imm]?
  if (Addr.getOpcode() == ISD::ADD) {
    return SelectImm(Addr.getOperand(0), Base) &&
           SelectImm(Addr.getOperand(1), Offset);
  }

  // is [imm]?
  if (SelectImm(Addr, Base)) {
    Offset = CurDAG->getTargetConstant(0, MVT::i32);
    return true;
  }

  return false;
}

bool MapipDAGToDAGISel::isImm(const SDValue &operand) {
  return ConstantSDNode::classof(operand.getNode());
}

bool MapipDAGToDAGISel::SelectImm(const SDValue &operand, SDValue &imm) {
  SDNode *node = operand.getNode();
  if (!ConstantSDNode::classof(node))
    return false;

  ConstantSDNode *CN = cast<ConstantSDNode>(node);
  imm = CurDAG->getTargetConstant(*CN->getConstantIntValue(), MVT::i32);
  return true;
}

const MapipSubtarget& MapipDAGToDAGISel::getSubtarget() const
{
  return TM.getSubtarget<MapipSubtarget>();
}

