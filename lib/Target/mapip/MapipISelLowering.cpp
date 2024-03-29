//===-- MapipISelLowering.cpp - Mapip DAG Lowering Implementation ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the MapipISelLowering class.
//
//===----------------------------------------------------------------------===//

#include "MapipISelLowering.h"
#include "MapipTargetMachine.h"
#include "MapipMachineFunctionInfo.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/PseudoSourceValue.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/Constants.h"
#include "llvm/Function.h"
#include "llvm/Module.h"
#include "llvm/Intrinsics.h"
#include "llvm/Type.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

/// AddLiveIn - This helper function adds the specified physical register to the
/// MachineFunction as a live in value.  It also creates a corresponding virtual
/// register for it.
static unsigned AddLiveIn(MachineFunction &MF, unsigned PReg,
                          TargetRegisterClass *RC) {
  assert(RC->contains(PReg) && "Not the correct regclass!");
  unsigned VReg = MF.getRegInfo().createVirtualRegister(RC);
  MF.getRegInfo().addLiveIn(PReg, VReg);
  return VReg;
}

MapipTargetLowering::MapipTargetLowering(TargetMachine &TM)
  : TargetLowering(TM, new TargetLoweringObjectFileELF()) {
  // Set up the TargetLowering object.
  //I am having problems with shr n i8 1
  setBooleanContents(ZeroOrOneBooleanContent);
  setBooleanVectorContents(ZeroOrOneBooleanContent); // FIXME: Is this correct?

  addRegisterClass(MVT::i32, Mapip::GPRCRegisterClass);
  addRegisterClass(MVT::f64, Mapip::F8RCRegisterClass);

  // We want to custom lower some of our intrinsics.
  setOperationAction(ISD::INTRINSIC_WO_CHAIN, MVT::Other, Custom);

  setLoadExtAction(ISD::EXTLOAD, MVT::i1,  Promote);
  setLoadExtAction(ISD::EXTLOAD, MVT::f64, Expand);

  setLoadExtAction(ISD::ZEXTLOAD, MVT::i1,  Promote);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::i32, Expand);

  setLoadExtAction(ISD::SEXTLOAD, MVT::i1,  Promote);
  setLoadExtAction(ISD::SEXTLOAD, MVT::i8,  Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::i16, Expand);

  //  setOperationAction(ISD::BRIND,        MVT::Other,   Expand);
  setOperationAction(ISD::BR_JT,        MVT::Other, Expand);
  setOperationAction(ISD::BR_CC,        MVT::Other, Expand);
  setOperationAction(ISD::SELECT_CC,    MVT::Other, Expand);

  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1, Expand);

  setOperationAction(ISD::FREM, MVT::f64, Expand);

  setOperationAction(ISD::UINT_TO_FP, MVT::i32, Expand);
  setOperationAction(ISD::SINT_TO_FP, MVT::i32, Custom);
  setOperationAction(ISD::FP_TO_UINT, MVT::i32, Expand);
  setOperationAction(ISD::FP_TO_SINT, MVT::i32, Custom);

  if (!TM.getSubtarget<MapipSubtarget>().hasCT()) {
    setOperationAction(ISD::CTPOP    , MVT::i32  , Expand);
    setOperationAction(ISD::CTTZ     , MVT::i32  , Expand);
    setOperationAction(ISD::CTLZ     , MVT::i32  , Expand);
  }
  setOperationAction(ISD::BSWAP    , MVT::i32, Expand);
  setOperationAction(ISD::ROTL     , MVT::i32, Expand);
  setOperationAction(ISD::ROTR     , MVT::i32, Expand);

  setOperationAction(ISD::SREM     , MVT::i32, Custom);
  setOperationAction(ISD::UREM     , MVT::i32, Custom);
  setOperationAction(ISD::SDIV     , MVT::i32, Custom);
  setOperationAction(ISD::UDIV     , MVT::i32, Custom);

  setOperationAction(ISD::ADDC     , MVT::i32, Expand);
  setOperationAction(ISD::ADDE     , MVT::i32, Expand);
  setOperationAction(ISD::SUBC     , MVT::i32, Expand);
  setOperationAction(ISD::SUBE     , MVT::i32, Expand);

  setOperationAction(ISD::SRL_PARTS, MVT::i32, Custom);
  setOperationAction(ISD::SRA_PARTS, MVT::i32, Expand);
  setOperationAction(ISD::SHL_PARTS, MVT::i32, Expand);

  // We don't support sin/cos/sqrt/pow
  setOperationAction(ISD::FSIN , MVT::f64, Expand);
  setOperationAction(ISD::FCOS , MVT::f64, Expand);
  setOperationAction(ISD::FSIN , MVT::f32, Expand);
  setOperationAction(ISD::FCOS , MVT::f32, Expand);

  setOperationAction(ISD::FSQRT, MVT::f64, Expand);
  setOperationAction(ISD::FSQRT, MVT::f32, Expand);

  setOperationAction(ISD::FPOW , MVT::f32, Expand);
  setOperationAction(ISD::FPOW , MVT::f64, Expand);

  setOperationAction(ISD::FMA, MVT::f64, Expand);

  setOperationAction(ISD::SETCC, MVT::f64, Promote);

  setOperationAction(ISD::BITCAST, MVT::f64, Promote);

  setOperationAction(ISD::EH_LABEL, MVT::Other, Expand);

  // Not implemented yet.
  setOperationAction(ISD::STACKSAVE, MVT::Other, Expand);
  setOperationAction(ISD::STACKRESTORE, MVT::Other, Expand);
  setOperationAction(ISD::DYNAMIC_STACKALLOC, MVT::i32, Expand);

  // We want to legalize GlobalAddress and ConstantPool and
  // ExternalSymbols nodes into the appropriate instructions to
  // materialize the address.
  setOperationAction(ISD::GlobalAddress,  MVT::i32, Custom);
  setOperationAction(ISD::ConstantPool,   MVT::i32, Custom);
  setOperationAction(ISD::ExternalSymbol, MVT::i32, Custom);
  setOperationAction(ISD::GlobalTLSAddress, MVT::i32, Custom);

  setOperationAction(ISD::VASTART, MVT::Other, Custom);
  setOperationAction(ISD::VAEND,   MVT::Other, Expand);
  setOperationAction(ISD::VACOPY,  MVT::Other, Custom);
  setOperationAction(ISD::VAARG,   MVT::Other, Custom);
  setOperationAction(ISD::VAARG,   MVT::i32,   Custom);

  setOperationAction(ISD::JumpTable, MVT::i32, Custom);

  setOperationAction(ISD::ATOMIC_LOAD,  MVT::i32, Expand);
  setOperationAction(ISD::ATOMIC_STORE, MVT::i32, Expand);

  setStackPointerRegisterToSaveRestore(Mapip::R30);

  setJumpBufSize(272);
  setJumpBufAlignment(16);

  setMinFunctionAlignment(4);

  setInsertFencesForAtomic(true);

  computeRegisterProperties();
}

EVT MapipTargetLowering::getSetCCResultType(EVT VT) const {
  return MVT::i32;
}

const char *MapipTargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (Opcode) {
  default: return 0;
  case MapipISD::CVTQT_: return "Mapip::CVTQT_";
  case MapipISD::CVTQS_: return "Mapip::CVTQS_";
  case MapipISD::CVTTQ_: return "Mapip::CVTTQ_";
  case MapipISD::GPRelHi: return "Mapip::GPRelHi";
  case MapipISD::GPRelLo: return "Mapip::GPRelLo";
  case MapipISD::RelLit: return "Mapip::RelLit";
  case MapipISD::GlobalRetAddr: return "Mapip::GlobalRetAddr";
  case MapipISD::CALL:   return "Mapip::CALL";
  case MapipISD::DivCall: return "Mapip::DivCall";
  case MapipISD::RET_FLAG: return "Mapip::RET_FLAG";
  case MapipISD::COND_BRANCH_I: return "Mapip::COND_BRANCH_I";
  case MapipISD::COND_BRANCH_F: return "Mapip::COND_BRANCH_F";
  }
}

static SDValue LowerJumpTable(SDValue Op, SelectionDAG &DAG) {
  EVT PtrVT = Op.getValueType();
  JumpTableSDNode *JT = cast<JumpTableSDNode>(Op);
  SDValue JTI = DAG.getTargetJumpTable(JT->getIndex(), PtrVT);
  // FIXME there isn't really any debug info here
  DebugLoc dl = Op.getDebugLoc();

  SDValue Hi = DAG.getNode(MapipISD::GPRelHi,  dl, MVT::i32, JTI,
                             DAG.getGLOBAL_OFFSET_TABLE(MVT::i32));
  SDValue Lo = DAG.getNode(MapipISD::GPRelLo, dl, MVT::i32, JTI, Hi);
  return Lo;
}

//http://www.cs.arizona.edu/computer.help/policy/DIGITAL_unix/
//AA-PY8AC-TET1_html/callCH3.html#BLOCK21

//For now, just use variable size stack frame format

//In a standard call, the first six items are passed in registers $16
//- $21 and/or registers $f16 - $f21. (See Section 4.1.2 for details
//of argument-to-register correspondence.) The remaining items are
//collected in a memory argument list that is a naturally aligned
//array of quadwords. In a standard call, this list, if present, must
//be passed at 0(SP).
//7 ... n         0(SP) ... (n-7)*8(SP)

// //#define FP    $15
// //#define RA    $26
// //#define PV    $27
// //#define GP    $29
// //#define SP    $30

#include "MapipGenCallingConv.inc"

SDValue
MapipTargetLowering::LowerCall(SDValue Chain, SDValue Callee,
                               CallingConv::ID CallConv, bool isVarArg,
                               bool &isTailCall,
                               const SmallVectorImpl<ISD::OutputArg> &Outs,
                               const SmallVectorImpl<SDValue> &OutVals,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               DebugLoc dl, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const {
  // Mapip target does not yet support tail call optimization.
  isTailCall = false;

  // Analyze operands of the call, assigning locations to each operand.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
		 getTargetMachine(), ArgLocs, *DAG.getContext());

  CCInfo.AnalyzeCallOperands(Outs, CC_Mapip);

    // Get a count of how many bytes are to be pushed on the stack.
  unsigned NumBytes = CCInfo.getNextStackOffset();

  Chain = DAG.getCALLSEQ_START(Chain, DAG.getConstant(NumBytes,
                                                      getPointerTy(), true));

  SmallVector<std::pair<unsigned, SDValue>, 4> RegsToPass;
  SmallVector<SDValue, 12> MemOpChains;
  SDValue StackPtr;

  // Walk the register/memloc assignments, inserting copies/loads.
  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];

    SDValue Arg = OutVals[i];

    // Promote the value if needed.
    switch (VA.getLocInfo()) {
      default: assert(0 && "Unknown loc info!");
      case CCValAssign::Full: break;
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

    // Arguments that can be passed on register must be kept at RegsToPass
    // vector
    if (VA.isRegLoc()) {
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
    } else {
      assert(VA.isMemLoc());

      if (StackPtr.getNode() == 0)
        StackPtr = DAG.getCopyFromReg(Chain, dl, Mapip::R30, MVT::i32);

      SDValue PtrOff = DAG.getNode(ISD::ADD, dl, getPointerTy(),
                                   StackPtr,
                                   DAG.getIntPtrConstant(VA.getLocMemOffset()));

      MemOpChains.push_back(DAG.getStore(Chain, dl, Arg, PtrOff,
                                         MachinePointerInfo(),false, false, 0));
    }
  }

  // Transform all store nodes into one single node because all store nodes are
  // independent of each other.
  if (!MemOpChains.empty())
    Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other,
                        &MemOpChains[0], MemOpChains.size());

  // Build a sequence of copy-to-reg nodes chained together with token chain and
  // flag operands which copy the outgoing args into registers.  The InFlag in
  // necessary since all emitted instructions must be stuck together.
  SDValue InFlag;
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i) {
    Chain = DAG.getCopyToReg(Chain, dl, RegsToPass[i].first,
                             RegsToPass[i].second, InFlag);
    InFlag = Chain.getValue(1);
  }

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

  Chain = DAG.getNode(MapipISD::CALL, dl, NodeTys, &Ops[0], Ops.size());
  InFlag = Chain.getValue(1);

  // Create the CALLSEQ_END node.
  Chain = DAG.getCALLSEQ_END(Chain,
                             DAG.getConstant(NumBytes, getPointerTy(), true),
                             DAG.getConstant(0, getPointerTy(), true),
                             InFlag);
  InFlag = Chain.getValue(1);

  // Handle result values, copying them out of physregs into vregs that we
  // return.
  return LowerCallResult(Chain, InFlag, CallConv, isVarArg,
                         Ins, dl, DAG, InVals);
}

/// LowerCallResult - Lower the result values of a call into the
/// appropriate copies out of appropriate physical registers.
///
SDValue
MapipTargetLowering::LowerCallResult(SDValue Chain, SDValue InFlag,
                                     CallingConv::ID CallConv, bool isVarArg,
                                     const SmallVectorImpl<ISD::InputArg> &Ins,
                                     DebugLoc dl, SelectionDAG &DAG,
                                     SmallVectorImpl<SDValue> &InVals) const {

  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
		 getTargetMachine(), RVLocs, *DAG.getContext());

  CCInfo.AnalyzeCallResult(Ins, RetCC_Mapip);

  // Copy all of the result registers out of their specified physreg.
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    CCValAssign &VA = RVLocs[i];

    Chain = DAG.getCopyFromReg(Chain, dl, VA.getLocReg(),
                               VA.getLocVT(), InFlag).getValue(1);
    SDValue RetValue = Chain.getValue(0);
    InFlag = Chain.getValue(2);

    // If this is an 8/16/32-bit value, it is really passed promoted to 64
    // bits. Insert an assert[sz]ext to capture this, then truncate to the
    // right size.
    if (VA.getLocInfo() == CCValAssign::SExt)
      RetValue = DAG.getNode(ISD::AssertSext, dl, VA.getLocVT(), RetValue,
                             DAG.getValueType(VA.getValVT()));
    else if (VA.getLocInfo() == CCValAssign::ZExt)
      RetValue = DAG.getNode(ISD::AssertZext, dl, VA.getLocVT(), RetValue,
                             DAG.getValueType(VA.getValVT()));

    if (VA.getLocInfo() != CCValAssign::Full)
      RetValue = DAG.getNode(ISD::TRUNCATE, dl, VA.getValVT(), RetValue);

    InVals.push_back(RetValue);
  }

  return Chain;
}

SDValue
MapipTargetLowering::LowerFormalArguments(SDValue Chain,
                                          CallingConv::ID CallConv, bool isVarArg,
                                          const SmallVectorImpl<ISD::InputArg>
                                            &Ins,
                                          DebugLoc dl, SelectionDAG &DAG,
                                          SmallVectorImpl<SDValue> &InVals)
                                            const {

  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo *MFI = MF.getFrameInfo();
  MapipMachineFunctionInfo *FuncInfo = MF.getInfo<MapipMachineFunctionInfo>();

  unsigned args_int[] = {
    Mapip::R16, Mapip::R17, Mapip::R18, Mapip::R19, Mapip::R20, Mapip::R21};
  unsigned args_float[] = {
    Mapip::F16, Mapip::F17, Mapip::F18, Mapip::F19, Mapip::F20, Mapip::F21};

  for (unsigned ArgNo = 0, e = Ins.size(); ArgNo != e; ++ArgNo) {
    SDValue argt;
    EVT ObjectVT = Ins[ArgNo].VT;
    SDValue ArgVal;

    if (ArgNo  < 6) {
      switch (ObjectVT.getSimpleVT().SimpleTy) {
      default:
        assert(false && "Invalid value type!");
      case MVT::f64:
        args_float[ArgNo] = AddLiveIn(MF, args_float[ArgNo],
                                      &Mapip::F8RCRegClass);
        ArgVal = DAG.getCopyFromReg(Chain, dl, args_float[ArgNo], ObjectVT);
        break;
      case MVT::i32:
        args_int[ArgNo] = AddLiveIn(MF, args_int[ArgNo],
                                    &Mapip::GPRCRegClass);
        ArgVal = DAG.getCopyFromReg(Chain, dl, args_int[ArgNo], MVT::i32);
        break;
      }
    } else { //more args
      // Create the frame index object for this incoming parameter...
      int FI = MFI->CreateFixedObject(8, 8 * (ArgNo - 6), true);

      // Create the SelectionDAG nodes corresponding to a load
      //from this parameter
      SDValue FIN = DAG.getFrameIndex(FI, MVT::i32);
      ArgVal = DAG.getLoad(ObjectVT, dl, Chain, FIN, MachinePointerInfo(),
                           false, false, 0);
    }
    InVals.push_back(ArgVal);
  }

  // If the functions takes variable number of arguments, copy all regs to stack
  if (isVarArg) {
    FuncInfo->setVarArgsOffset(Ins.size() * 8);
    std::vector<SDValue> LS;
    for (int i = 0; i < 6; ++i) {
      if (TargetRegisterInfo::isPhysicalRegister(args_int[i]))
        args_int[i] = AddLiveIn(MF, args_int[i], &Mapip::GPRCRegClass);
      SDValue argt = DAG.getCopyFromReg(Chain, dl, args_int[i], MVT::i32);
      int FI = MFI->CreateFixedObject(8, -8 * (6 - i), true);
      if (i == 0) FuncInfo->setVarArgsBase(FI);
      SDValue SDFI = DAG.getFrameIndex(FI, MVT::i32);
      LS.push_back(DAG.getStore(Chain, dl, argt, SDFI, MachinePointerInfo(),
                                false, false, 0));

      if (TargetRegisterInfo::isPhysicalRegister(args_float[i]))
        args_float[i] = AddLiveIn(MF, args_float[i], &Mapip::F8RCRegClass);
      argt = DAG.getCopyFromReg(Chain, dl, args_float[i], MVT::f64);
      FI = MFI->CreateFixedObject(8, - 8 * (12 - i), true);
      SDFI = DAG.getFrameIndex(FI, MVT::i32);
      LS.push_back(DAG.getStore(Chain, dl, argt, SDFI, MachinePointerInfo(),
                                false, false, 0));
    }

    //Set up a token factor with all the stack traffic
    Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other, &LS[0], LS.size());
  }

  return Chain;
}

SDValue
MapipTargetLowering::LowerReturn(SDValue Chain,
                                 CallingConv::ID CallConv, bool isVarArg,
                                 const SmallVectorImpl<ISD::OutputArg> &Outs,
                                 const SmallVectorImpl<SDValue> &OutVals,
                                 DebugLoc dl, SelectionDAG &DAG) const {

  SDValue Copy = DAG.getCopyToReg(Chain, dl, Mapip::R26,
                                  DAG.getNode(MapipISD::GlobalRetAddr,
                                              DebugLoc(), MVT::i32),
                                  SDValue());
  switch (Outs.size()) {
  default:
    llvm_unreachable("Do not know how to return this many arguments!");
  case 0:
    break;
    //return SDValue(); // ret void is legal
  case 1: {
    EVT ArgVT = Outs[0].VT;
    unsigned ArgReg;
    if (ArgVT.isInteger())
      ArgReg = Mapip::R0;
    else {
      assert(ArgVT.isFloatingPoint());
      ArgReg = Mapip::F0;
    }
    Copy = DAG.getCopyToReg(Copy, dl, ArgReg,
                            OutVals[0], Copy.getValue(1));
    if (DAG.getMachineFunction().getRegInfo().liveout_empty())
      DAG.getMachineFunction().getRegInfo().addLiveOut(ArgReg);
    break;
  }
  case 2: {
    EVT ArgVT = Outs[0].VT;
    unsigned ArgReg1, ArgReg2;
    if (ArgVT.isInteger()) {
      ArgReg1 = Mapip::R0;
      ArgReg2 = Mapip::R1;
    } else {
      assert(ArgVT.isFloatingPoint());
      ArgReg1 = Mapip::F0;
      ArgReg2 = Mapip::F1;
    }
    Copy = DAG.getCopyToReg(Copy, dl, ArgReg1,
                            OutVals[0], Copy.getValue(1));
    if (std::find(DAG.getMachineFunction().getRegInfo().liveout_begin(),
                  DAG.getMachineFunction().getRegInfo().liveout_end(), ArgReg1)
        == DAG.getMachineFunction().getRegInfo().liveout_end())
      DAG.getMachineFunction().getRegInfo().addLiveOut(ArgReg1);
    Copy = DAG.getCopyToReg(Copy, dl, ArgReg2,
                            OutVals[1], Copy.getValue(1));
    if (std::find(DAG.getMachineFunction().getRegInfo().liveout_begin(),
                   DAG.getMachineFunction().getRegInfo().liveout_end(), ArgReg2)
        == DAG.getMachineFunction().getRegInfo().liveout_end())
      DAG.getMachineFunction().getRegInfo().addLiveOut(ArgReg2);
    break;
  }
  }
  return DAG.getNode(MapipISD::RET_FLAG, dl,
                     MVT::Other, Copy, Copy.getValue(1));
}

void MapipTargetLowering::LowerVAARG(SDNode *N, SDValue &Chain,
                                     SDValue &DataPtr,
                                     SelectionDAG &DAG) const {
  Chain = N->getOperand(0);
  SDValue VAListP = N->getOperand(1);
  const Value *VAListS = cast<SrcValueSDNode>(N->getOperand(2))->getValue();
  DebugLoc dl = N->getDebugLoc();

  SDValue Base = DAG.getLoad(MVT::i32, dl, Chain, VAListP,
                             MachinePointerInfo(VAListS),
                             false, false, 0);
  SDValue Tmp = DAG.getNode(ISD::ADD, dl, MVT::i32, VAListP,
                              DAG.getConstant(4, MVT::i32));
  SDValue Offset = DAG.getExtLoad(ISD::SEXTLOAD, dl, MVT::i32, Base.getValue(1),
                                  Tmp, MachinePointerInfo(),
                                  MVT::i32, false, false, 0);
  DataPtr = DAG.getNode(ISD::ADD, dl, MVT::i32, Base, Offset);
  if (N->getValueType(0).isFloatingPoint())
  {
    //if fp && Offset < 6*4, then subtract 6*4 from DataPtr
    SDValue FPDataPtr = DAG.getNode(ISD::SUB, dl, MVT::i32, DataPtr,
                                      DAG.getConstant(4*6, MVT::i32));
    SDValue CC = DAG.getSetCC(dl, MVT::i32, Offset,
                                DAG.getConstant(4*6, MVT::i32), ISD::SETLT);
    DataPtr = DAG.getNode(ISD::SELECT, dl, MVT::i32, CC, FPDataPtr, DataPtr);
  }

  SDValue NewOffset = DAG.getNode(ISD::ADD, dl, MVT::i32, Offset,
                                    DAG.getConstant(4, MVT::i32));
  Chain = DAG.getTruncStore(Offset.getValue(1), dl, NewOffset, Tmp,
                            MachinePointerInfo(),
                            MVT::i32, false, false, 0);
}

/// LowerOperation - Provide custom lowering hooks for some operations.
///
SDValue MapipTargetLowering::LowerOperation(SDValue Op,
                                            SelectionDAG &DAG) const {
  DebugLoc dl = Op.getDebugLoc();
  switch (Op.getOpcode()) {
  default: llvm_unreachable("Wasn't expecting to be able to lower this!");
  case ISD::JumpTable: return LowerJumpTable(Op, DAG);

  case ISD::INTRINSIC_WO_CHAIN: {
    unsigned IntNo = cast<ConstantSDNode>(Op.getOperand(0))->getZExtValue();
    switch (IntNo) {
    default: break;    // Don't custom lower most intrinsics.
    case Intrinsic::alpha_umulh:
      return DAG.getNode(ISD::MULHU, dl, MVT::i32,
                         Op.getOperand(1), Op.getOperand(2));
    }
  }

  case ISD::SRL_PARTS: {
    SDValue ShOpLo = Op.getOperand(0);
    SDValue ShOpHi = Op.getOperand(1);
    SDValue ShAmt  = Op.getOperand(2);
    SDValue bm = DAG.getNode(ISD::SUB, dl, MVT::i32,
                             DAG.getConstant(64, MVT::i32), ShAmt);
    SDValue BMCC = DAG.getSetCC(dl, MVT::i32, bm,
                                DAG.getConstant(0, MVT::i32), ISD::SETLE);
    // if 64 - shAmt <= 0
    SDValue Hi_Neg = DAG.getConstant(0, MVT::i32);
    SDValue ShAmt_Neg = DAG.getNode(ISD::SUB, dl, MVT::i32,
                                    DAG.getConstant(0, MVT::i32), bm);
    SDValue Lo_Neg = DAG.getNode(ISD::SRL, dl, MVT::i32, ShOpHi, ShAmt_Neg);
    // else
    SDValue carries = DAG.getNode(ISD::SHL, dl, MVT::i32, ShOpHi, bm);
    SDValue Hi_Pos =  DAG.getNode(ISD::SRL, dl, MVT::i32, ShOpHi, ShAmt);
    SDValue Lo_Pos = DAG.getNode(ISD::SRL, dl, MVT::i32, ShOpLo, ShAmt);
    Lo_Pos = DAG.getNode(ISD::OR, dl, MVT::i32, Lo_Pos, carries);
    // Merge
    SDValue Hi = DAG.getNode(ISD::SELECT, dl, MVT::i32, BMCC, Hi_Neg, Hi_Pos);
    SDValue Lo = DAG.getNode(ISD::SELECT, dl, MVT::i32, BMCC, Lo_Neg, Lo_Pos);
    SDValue Ops[2] = { Lo, Hi };
    return DAG.getMergeValues(Ops, 2, dl);
  }
    //  case ISD::SRA_PARTS:

    //  case ISD::SHL_PARTS:


  case ISD::SINT_TO_FP: {
    assert(Op.getOperand(0).getValueType() == MVT::i32 &&
           "Unhandled SINT_TO_FP type in custom expander!");
    SDValue LD;
    bool isDouble = Op.getValueType() == MVT::f64;
    LD = DAG.getNode(ISD::BITCAST, dl, MVT::f64, Op.getOperand(0));
    SDValue FP = DAG.getNode(isDouble?MapipISD::CVTQT_:MapipISD::CVTQS_, dl,
                               isDouble?MVT::f64:MVT::f32, LD);
    return FP;
  }
  case ISD::FP_TO_SINT: {
    bool isDouble = Op.getOperand(0).getValueType() == MVT::f64;
    SDValue src = Op.getOperand(0);

    if (!isDouble) //Promote
      src = DAG.getNode(ISD::FP_EXTEND, dl, MVT::f64, src);

    src = DAG.getNode(MapipISD::CVTTQ_, dl, MVT::f64, src);

    return DAG.getNode(ISD::BITCAST, dl, MVT::i32, src);
  }
  case ISD::ConstantPool: {
    ConstantPoolSDNode *CP = cast<ConstantPoolSDNode>(Op);
    const Constant *C = CP->getConstVal();
    SDValue CPI = DAG.getTargetConstantPool(C, MVT::i32, CP->getAlignment());
    // FIXME there isn't really any debug info here

    SDValue Hi = DAG.getNode(MapipISD::GPRelHi,  dl, MVT::i32, CPI,
                               DAG.getGLOBAL_OFFSET_TABLE(MVT::i32));
    SDValue Lo = DAG.getNode(MapipISD::GPRelLo, dl, MVT::i32, CPI, Hi);
    return Lo;
  }
  case ISD::GlobalTLSAddress:
    llvm_unreachable("TLS not implemented for Mapip.");
  case ISD::GlobalAddress: {
    GlobalAddressSDNode *GSDN = cast<GlobalAddressSDNode>(Op);
    const GlobalValue *GV = GSDN->getGlobal();
    SDValue GA = DAG.getTargetGlobalAddress(GV, dl, MVT::i32,
                                            GSDN->getOffset());
    // FIXME there isn't really any debug info here

    //    if (!GV->hasWeakLinkage() && !GV->isDeclaration()
    //        && !GV->hasLinkOnceLinkage()) {
    if (GV->hasLocalLinkage()) {
      SDValue Hi = DAG.getNode(MapipISD::GPRelHi,  dl, MVT::i32, GA,
                                DAG.getGLOBAL_OFFSET_TABLE(MVT::i32));
      SDValue Lo = DAG.getNode(MapipISD::GPRelLo, dl, MVT::i32, GA, Hi);
      return Lo;
    } else
      return DAG.getNode(MapipISD::RelLit, dl, MVT::i32, GA,
                         DAG.getGLOBAL_OFFSET_TABLE(MVT::i32));
  }
  case ISD::ExternalSymbol: {
    return DAG.getNode(MapipISD::RelLit, dl, MVT::i32,
                       DAG.getTargetExternalSymbol(cast<ExternalSymbolSDNode>(Op)
                                                   ->getSymbol(), MVT::i32),
                       DAG.getGLOBAL_OFFSET_TABLE(MVT::i32));
  }

  case ISD::UREM:
  case ISD::SREM:
    //Expand only on constant case
    if (Op.getOperand(1).getOpcode() == ISD::Constant) {
      EVT VT = Op.getNode()->getValueType(0);
      SDValue Tmp1 = Op.getNode()->getOpcode() == ISD::UREM ?
        BuildUDIV(Op.getNode(), DAG, NULL) :
        BuildSDIV(Op.getNode(), DAG, NULL);
      Tmp1 = DAG.getNode(ISD::MUL, dl, VT, Tmp1, Op.getOperand(1));
      Tmp1 = DAG.getNode(ISD::SUB, dl, VT, Op.getOperand(0), Tmp1);
      return Tmp1;
    }
    //fall through
  case ISD::SDIV:
  case ISD::UDIV:
    if (Op.getValueType().isInteger()) {
      if (Op.getOperand(1).getOpcode() == ISD::Constant)
        return Op.getOpcode() == ISD::SDIV ? BuildSDIV(Op.getNode(), DAG, NULL)
          : BuildUDIV(Op.getNode(), DAG, NULL);
      const char* opstr = 0;
      switch (Op.getOpcode()) {
      case ISD::UREM: opstr = "__remqu"; break;
      case ISD::SREM: opstr = "__remq";  break;
      case ISD::UDIV: opstr = "__divqu"; break;
      case ISD::SDIV: opstr = "__divq";  break;
      }
      SDValue Tmp1 = Op.getOperand(0),
        Tmp2 = Op.getOperand(1),
        Addr = DAG.getExternalSymbol(opstr, MVT::i32);
      return DAG.getNode(MapipISD::DivCall, dl, MVT::i32, Addr, Tmp1, Tmp2);
    }
    break;

  case ISD::VAARG: {
    SDValue Chain, DataPtr;
    LowerVAARG(Op.getNode(), Chain, DataPtr, DAG);

    SDValue Result;
    if (Op.getValueType() == MVT::i32)
      Result = DAG.getExtLoad(ISD::SEXTLOAD, dl, MVT::i32, Chain, DataPtr,
                              MachinePointerInfo(), MVT::i32, false, false, 0);
    else
      Result = DAG.getLoad(Op.getValueType(), dl, Chain, DataPtr,
                           MachinePointerInfo(),
                           false, false, 0);
    return Result;
  }
  case ISD::VACOPY: {
    SDValue Chain = Op.getOperand(0);
    SDValue DestP = Op.getOperand(1);
    SDValue SrcP = Op.getOperand(2);
    const Value *DestS = cast<SrcValueSDNode>(Op.getOperand(3))->getValue();
    const Value *SrcS = cast<SrcValueSDNode>(Op.getOperand(4))->getValue();

    SDValue Val = DAG.getLoad(getPointerTy(), dl, Chain, SrcP,
                              MachinePointerInfo(SrcS),
                              false, false, 0);
    SDValue Result = DAG.getStore(Val.getValue(1), dl, Val, DestP,
                                  MachinePointerInfo(DestS),
                                  false, false, 0);
    SDValue NP = DAG.getNode(ISD::ADD, dl, MVT::i32, SrcP,
                               DAG.getConstant(8, MVT::i32));
    Val = DAG.getExtLoad(ISD::SEXTLOAD, dl, MVT::i32, Result,
                         NP, MachinePointerInfo(), MVT::i32, false, false, 0);
    SDValue NPD = DAG.getNode(ISD::ADD, dl, MVT::i32, DestP,
                                DAG.getConstant(8, MVT::i32));
    return DAG.getTruncStore(Val.getValue(1), dl, Val, NPD,
                             MachinePointerInfo(), MVT::i32,
                             false, false, 0);
  }
  case ISD::VASTART: {
    MachineFunction &MF = DAG.getMachineFunction();
    MapipMachineFunctionInfo *FuncInfo = MF.getInfo<MapipMachineFunctionInfo>();

    SDValue Chain = Op.getOperand(0);
    SDValue VAListP = Op.getOperand(1);
    const Value *VAListS = cast<SrcValueSDNode>(Op.getOperand(2))->getValue();

    // vastart stores the address of the VarArgsBase and VarArgsOffset
    SDValue FR  = DAG.getFrameIndex(FuncInfo->getVarArgsBase(), MVT::i32);
    SDValue S1  = DAG.getStore(Chain, dl, FR, VAListP,
                               MachinePointerInfo(VAListS), false, false, 0);
    SDValue SA2 = DAG.getNode(ISD::ADD, dl, MVT::i32, VAListP,
                                DAG.getConstant(8, MVT::i32));
    return DAG.getTruncStore(S1, dl,
                             DAG.getConstant(FuncInfo->getVarArgsOffset(),
                                             MVT::i32),
                             SA2, MachinePointerInfo(),
                             MVT::i32, false, false, 0);
  }
  case ISD::RETURNADDR:
    return DAG.getNode(MapipISD::GlobalRetAddr, DebugLoc(), MVT::i32);
      //FIXME: implement
  case ISD::FRAMEADDR:          break;
  }

  return SDValue();
}

void MapipTargetLowering::ReplaceNodeResults(SDNode *N,
                                             SmallVectorImpl<SDValue>&Results,
                                             SelectionDAG &DAG) const {
  DebugLoc dl = N->getDebugLoc();
  assert(N->getValueType(0) == MVT::i32 &&
         N->getOpcode() == ISD::VAARG &&
         "Unknown node to custom promote!");

  SDValue Chain, DataPtr;
  LowerVAARG(N, Chain, DataPtr, DAG);
  SDValue Res = DAG.getLoad(N->getValueType(0), dl, Chain, DataPtr,
                            MachinePointerInfo(),
                            false, false, 0);
  Results.push_back(Res);
  Results.push_back(SDValue(Res.getNode(), 1));
}


//Inline Asm

/// getConstraintType - Given a constraint letter, return the type of
/// constraint it is for this target.
MapipTargetLowering::ConstraintType
MapipTargetLowering::getConstraintType(const std::string &Constraint) const {
  if (Constraint.size() == 1) {
    switch (Constraint[0]) {
    default: break;
    case 'f':
    case 'r':
      return C_RegisterClass;
    }
  }
  return TargetLowering::getConstraintType(Constraint);
}

/// Examine constraint type and operand type and determine a weight value.
/// This object must already have been set up with the operand type
/// and the current alternative constraint selected.
TargetLowering::ConstraintWeight
MapipTargetLowering::getSingleConstraintMatchWeight(
    AsmOperandInfo &info, const char *constraint) const {
  ConstraintWeight weight = CW_Invalid;
  Value *CallOperandVal = info.CallOperandVal;
    // If we don't have a value, we can't do a match,
    // but allow it at the lowest weight.
  if (CallOperandVal == NULL)
    return CW_Default;
  // Look at the constraint type.
  switch (*constraint) {
  default:
    weight = TargetLowering::getSingleConstraintMatchWeight(info, constraint);
    break;
  case 'f':
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
    case 'r':
      return std::make_pair(0U, Mapip::GPRCRegisterClass);
    case 'f':
      return std::make_pair(0U, Mapip::F8RCRegisterClass);
    }
  }
  return TargetLowering::getRegForInlineAsmConstraint(Constraint, VT);
}

//===----------------------------------------------------------------------===//
//  Other Lowering Code
//===----------------------------------------------------------------------===//

MachineBasicBlock *
MapipTargetLowering::EmitInstrWithCustomInserter(MachineInstr *MI,
                                                 MachineBasicBlock *BB) const {
  const TargetInstrInfo *TII = getTargetMachine().getInstrInfo();
  assert((MI->getOpcode() == Mapip::CAS32 ||
          MI->getOpcode() == Mapip::CAS64 ||
          MI->getOpcode() == Mapip::LAS32 ||
          MI->getOpcode() == Mapip::LAS64 ||
          MI->getOpcode() == Mapip::SWAP32 ||
          MI->getOpcode() == Mapip::SWAP64) &&
         "Unexpected instr type to insert");

  bool is32 = MI->getOpcode() == Mapip::CAS32 ||
    MI->getOpcode() == Mapip::LAS32 ||
    MI->getOpcode() == Mapip::SWAP32;

  //Load locked store conditional for atomic ops take on the same form
  //start:
  //ll
  //do stuff (maybe branch to exit)
  //sc
  //test sc and maybe branck to start
  //exit:
  const BasicBlock *LLVM_BB = BB->getBasicBlock();
  DebugLoc dl = MI->getDebugLoc();
  MachineFunction::iterator It = BB;
  ++It;

  MachineBasicBlock *thisMBB = BB;
  MachineFunction *F = BB->getParent();
  MachineBasicBlock *llscMBB = F->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *sinkMBB = F->CreateMachineBasicBlock(LLVM_BB);

  sinkMBB->splice(sinkMBB->begin(), thisMBB,
                  llvm::next(MachineBasicBlock::iterator(MI)),
                  thisMBB->end());
  sinkMBB->transferSuccessorsAndUpdatePHIs(thisMBB);

  F->insert(It, llscMBB);
  F->insert(It, sinkMBB);

  BuildMI(thisMBB, dl, TII->get(Mapip::BR)).addMBB(llscMBB);

  unsigned reg_res = MI->getOperand(0).getReg(),
    reg_ptr = MI->getOperand(1).getReg(),
    reg_v2 = MI->getOperand(2).getReg(),
    reg_store = F->getRegInfo().createVirtualRegister(&Mapip::GPRCRegClass);

  BuildMI(llscMBB, dl, TII->get(is32 ? Mapip::LDL_L : Mapip::LDQ_L),
          reg_res).addImm(0).addReg(reg_ptr);
  switch (MI->getOpcode()) {
  case Mapip::CAS32:
  case Mapip::CAS64: {
    unsigned reg_cmp
      = F->getRegInfo().createVirtualRegister(&Mapip::GPRCRegClass);
    BuildMI(llscMBB, dl, TII->get(Mapip::CMPEQ), reg_cmp)
      .addReg(reg_v2).addReg(reg_res);
    BuildMI(llscMBB, dl, TII->get(Mapip::BEQ))
      .addImm(0).addReg(reg_cmp).addMBB(sinkMBB);
    BuildMI(llscMBB, dl, TII->get(Mapip::BISr), reg_store)
      .addReg(Mapip::R31).addReg(MI->getOperand(3).getReg());
    break;
  }
  case Mapip::LAS32:
  case Mapip::LAS64: {
    BuildMI(llscMBB, dl, TII->get(Mapip::ADDLr), reg_store)
      .addReg(reg_res).addReg(reg_v2);
    break;
  }
  case Mapip::SWAP32:
  case Mapip::SWAP64: {
    BuildMI(llscMBB, dl, TII->get(Mapip::BISr), reg_store)
      .addReg(reg_v2).addReg(reg_v2);
    break;
  }
  }
  BuildMI(llscMBB, dl, TII->get(is32 ? Mapip::STL_C : Mapip::STQ_C), reg_store)
    .addReg(reg_store).addImm(0).addReg(reg_ptr);
  BuildMI(llscMBB, dl, TII->get(Mapip::BEQ))
    .addImm(0).addReg(reg_store).addMBB(llscMBB);
  BuildMI(llscMBB, dl, TII->get(Mapip::BR)).addMBB(sinkMBB);

  thisMBB->addSuccessor(llscMBB);
  llscMBB->addSuccessor(llscMBB);
  llscMBB->addSuccessor(sinkMBB);
  MI->eraseFromParent();   // The pseudo instruction is gone now.

  return sinkMBB;
}

bool
MapipTargetLowering::isOffsetFoldingLegal(const GlobalAddressSDNode *GA) const {
  // The Mapip target isn't yet aware of offsets.
  return false;
}

bool MapipTargetLowering::isFPImmLegal(const APFloat &Imm, EVT VT) const {
  if (VT != MVT::f32 && VT != MVT::f64)
    return false;
  // +0.0   F31
  // +0.0f  F31
  // -0.0  -F31
  // -0.0f -F31
  return Imm.isZero() || Imm.isNegZero();
}
