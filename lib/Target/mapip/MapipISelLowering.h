//==-- MapipISelLowering.h - Mapip DAG Lowering Interface ------------*- C++ -*-==//
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

#ifndef Mapip_ISEL_LOWERING_H
#define Mapip_ISEL_LOWERING_H

#include "llvm/Target/TargetLowering.h"

namespace llvm {
class MapipSubtarget;
class MapipTargetMachine;

namespace MapipISD {
  enum NodeType {
    FIRST_NUMBER = ISD::BUILTIN_OP_END,
    READ_PARAM,
    EXIT,
    RET
  };
} // namespace MapipISD

class MapipTargetLowering : public TargetLowering {
  public:
    explicit MapipTargetLowering(TargetMachine &TM);

    virtual const char *getTargetNodeName(unsigned Opcode) const;

    virtual unsigned getFunctionAlignment(const Function *F) const {
      return 2; }

    virtual SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const;

    virtual SDValue
      LowerFormalArguments(SDValue Chain,
                           CallingConv::ID CallConv,
                           bool isVarArg,
                           const SmallVectorImpl<ISD::InputArg> &Ins,
                           DebugLoc dl,
                           SelectionDAG &DAG,
                           SmallVectorImpl<SDValue> &InVals) const;

    virtual SDValue
      LowerReturn(SDValue Chain,
                  CallingConv::ID CallConv,
                  bool isVarArg,
                  const SmallVectorImpl<ISD::OutputArg> &Outs,
                  const SmallVectorImpl<SDValue> &OutVals,
                  DebugLoc dl,
                  SelectionDAG &DAG) const;

  private:
    SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;
}; // class MapipTargetLowering
} // namespace llvm

#endif // Mapip_ISEL_LOWERING_H
