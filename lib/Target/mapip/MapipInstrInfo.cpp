//===- MapipInstrInfo.cpp - Mapip Instruction Information -------*- C++ -*-===//
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

#include "Mapip.h"
#include "MapipInstrInfo.h"
#include "MapipMachineFunctionInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/ErrorHandling.h"

#define GET_INSTRINFO_CTOR
#include "MapipGenInstrInfo.inc"
using namespace llvm;

MapipInstrInfo::MapipInstrInfo()
  : MapipGenInstrInfo(Mapip::ADJUSTSTACKDOWN, Mapip::ADJUSTSTACKUP),
    RI(*this) {
}


unsigned
MapipInstrInfo::isLoadFromStackSlot(const MachineInstr *MI,
                                    int &FrameIndex) const {
  switch (MI->getOpcode()) {
  case Mapip::LDL:
  case Mapip::LDQ:
  case Mapip::LDBU:
  case Mapip::LDWU:
  case Mapip::LDS:
  case Mapip::LDT:
    if (MI->getOperand(1).isFI()) {
      FrameIndex = MI->getOperand(1).getIndex();
      return MI->getOperand(0).getReg();
    }
    break;
  }
  return 0;
}

unsigned
MapipInstrInfo::isStoreToStackSlot(const MachineInstr *MI,
                                   int &FrameIndex) const {
  switch (MI->getOpcode()) {
  case Mapip::STL:
  case Mapip::STQ:
  case Mapip::STB:
  case Mapip::STW:
  case Mapip::STS:
  case Mapip::STT:
    if (MI->getOperand(1).isFI()) {
      FrameIndex = MI->getOperand(1).getIndex();
      return MI->getOperand(0).getReg();
    }
    break;
  }
  return 0;
}

static bool isMapipIntCondCode(unsigned Opcode) {
  switch (Opcode) {
  case Mapip::BEQ:
  case Mapip::BNE:
  case Mapip::BGE:
  case Mapip::BGT:
  case Mapip::BLE:
  case Mapip::BLT:
  case Mapip::BLBC:
  case Mapip::BLBS:
    return true;
  default:
    return false;
  }
}

unsigned MapipInstrInfo::InsertBranch(MachineBasicBlock &MBB,
                                      MachineBasicBlock *TBB,
                                      MachineBasicBlock *FBB,
                                      const SmallVectorImpl<MachineOperand> &Cond,
                                      DebugLoc DL) const {
  assert(TBB && "InsertBranch must not be told to insert a fallthrough");
  assert((Cond.size() == 2 || Cond.size() == 0) &&
         "Mapip branch conditions have two components!");

  // One-way branch.
  if (FBB == 0) {
    if (Cond.empty())   // Unconditional branch
      BuildMI(&MBB, DL, get(Mapip::BR)).addMBB(TBB);
    else                // Conditional branch
      if (isMapipIntCondCode(Cond[0].getImm()))
        BuildMI(&MBB, DL, get(Mapip::COND_BRANCH_I))
          .addImm(Cond[0].getImm()).addReg(Cond[1].getReg()).addMBB(TBB);
      else
        BuildMI(&MBB, DL, get(Mapip::COND_BRANCH_F))
          .addImm(Cond[0].getImm()).addReg(Cond[1].getReg()).addMBB(TBB);
    return 1;
  }

  // Two-way Conditional Branch.
  if (isMapipIntCondCode(Cond[0].getImm()))
    BuildMI(&MBB, DL, get(Mapip::COND_BRANCH_I))
      .addImm(Cond[0].getImm()).addReg(Cond[1].getReg()).addMBB(TBB);
  else
    BuildMI(&MBB, DL, get(Mapip::COND_BRANCH_F))
      .addImm(Cond[0].getImm()).addReg(Cond[1].getReg()).addMBB(TBB);
  BuildMI(&MBB, DL, get(Mapip::BR)).addMBB(FBB);
  return 2;
}

void MapipInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MI, DebugLoc DL,
                                 unsigned DestReg, unsigned SrcReg,
                                 bool KillSrc) const {
  if (Mapip::GPRCRegClass.contains(DestReg, SrcReg)) {
    BuildMI(MBB, MI, DL, get(Mapip::BISr), DestReg)
      .addReg(SrcReg)
      .addReg(SrcReg, getKillRegState(KillSrc));
  } else if (Mapip::F4RCRegClass.contains(DestReg, SrcReg)) {
    BuildMI(MBB, MI, DL, get(Mapip::CPYSS), DestReg)
      .addReg(SrcReg)
      .addReg(SrcReg, getKillRegState(KillSrc));
  } else if (Mapip::F8RCRegClass.contains(DestReg, SrcReg)) {
    BuildMI(MBB, MI, DL, get(Mapip::CPYST), DestReg)
      .addReg(SrcReg)
      .addReg(SrcReg, getKillRegState(KillSrc));
  } else {
    llvm_unreachable("Attempt to copy register that is not GPR or FPR");
  }
}

void
MapipInstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                    MachineBasicBlock::iterator MI,
                                    unsigned SrcReg, bool isKill, int FrameIdx,
                                    const TargetRegisterClass *RC,
                                    const TargetRegisterInfo *TRI) const {
  //cerr << "Trying to store " << getPrettyName(SrcReg) << " to "
  //     << FrameIdx << "\n";
  //BuildMI(MBB, MI, Mapip::WTF, 0).addReg(SrcReg);

  DebugLoc DL;
  if (MI != MBB.end()) DL = MI->getDebugLoc();

  if (RC == Mapip::F4RCRegisterClass)
    BuildMI(MBB, MI, DL, get(Mapip::STS))
      .addReg(SrcReg, getKillRegState(isKill))
      .addFrameIndex(FrameIdx).addReg(Mapip::F31);
  else if (RC == Mapip::F8RCRegisterClass)
    BuildMI(MBB, MI, DL, get(Mapip::STT))
      .addReg(SrcReg, getKillRegState(isKill))
      .addFrameIndex(FrameIdx).addReg(Mapip::F31);
  else if (RC == Mapip::GPRCRegisterClass)
    BuildMI(MBB, MI, DL, get(Mapip::STQ))
      .addReg(SrcReg, getKillRegState(isKill))
      .addFrameIndex(FrameIdx).addReg(Mapip::F31);
  else
    llvm_unreachable("Unhandled register class");
}

void
MapipInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                        MachineBasicBlock::iterator MI,
                                        unsigned DestReg, int FrameIdx,
                                     const TargetRegisterClass *RC,
                                     const TargetRegisterInfo *TRI) const {
  //cerr << "Trying to load " << getPrettyName(DestReg) << " to "
  //     << FrameIdx << "\n";
  DebugLoc DL;
  if (MI != MBB.end()) DL = MI->getDebugLoc();

  if (RC == Mapip::F4RCRegisterClass)
    BuildMI(MBB, MI, DL, get(Mapip::LDS), DestReg)
      .addFrameIndex(FrameIdx).addReg(Mapip::F31);
  else if (RC == Mapip::F8RCRegisterClass)
    BuildMI(MBB, MI, DL, get(Mapip::LDT), DestReg)
      .addFrameIndex(FrameIdx).addReg(Mapip::F31);
  else if (RC == Mapip::GPRCRegisterClass)
    BuildMI(MBB, MI, DL, get(Mapip::LDQ), DestReg)
      .addFrameIndex(FrameIdx).addReg(Mapip::F31);
  else
    llvm_unreachable("Unhandled register class");
}

static unsigned MapipRevCondCode(unsigned Opcode) {
  switch (Opcode) {
  case Mapip::BEQ: return Mapip::BNE;
  case Mapip::BNE: return Mapip::BEQ;
  case Mapip::BGE: return Mapip::BLT;
  case Mapip::BGT: return Mapip::BLE;
  case Mapip::BLE: return Mapip::BGT;
  case Mapip::BLT: return Mapip::BGE;
  case Mapip::BLBC: return Mapip::BLBS;
  case Mapip::BLBS: return Mapip::BLBC;
  case Mapip::FBEQ: return Mapip::FBNE;
  case Mapip::FBNE: return Mapip::FBEQ;
  case Mapip::FBGE: return Mapip::FBLT;
  case Mapip::FBGT: return Mapip::FBLE;
  case Mapip::FBLE: return Mapip::FBGT;
  case Mapip::FBLT: return Mapip::FBGE;
  default:
    llvm_unreachable("Unknown opcode");
  }
  return 0; // Not reached
}

// Branch analysis.
bool MapipInstrInfo::AnalyzeBranch(MachineBasicBlock &MBB,MachineBasicBlock *&TBB,
                                   MachineBasicBlock *&FBB,
                                   SmallVectorImpl<MachineOperand> &Cond,
                                   bool AllowModify) const {
  // If the block has no terminators, it just falls into the block after it.
  MachineBasicBlock::iterator I = MBB.end();
  if (I == MBB.begin())
    return false;
  --I;
  while (I->isDebugValue()) {
    if (I == MBB.begin())
      return false;
    --I;
  }
  if (!isUnpredicatedTerminator(I))
    return false;

  // Get the last instruction in the block.
  MachineInstr *LastInst = I;

  // If there is only one terminator instruction, process it.
  if (I == MBB.begin() || !isUnpredicatedTerminator(--I)) {
    if (LastInst->getOpcode() == Mapip::BR) {
      TBB = LastInst->getOperand(0).getMBB();
      return false;
    } else if (LastInst->getOpcode() == Mapip::COND_BRANCH_I ||
               LastInst->getOpcode() == Mapip::COND_BRANCH_F) {
      // Block ends with fall-through condbranch.
      TBB = LastInst->getOperand(2).getMBB();
      Cond.push_back(LastInst->getOperand(0));
      Cond.push_back(LastInst->getOperand(1));
      return false;
    }
    // Otherwise, don't know what this is.
    return true;
  }

  // Get the instruction before it if it's a terminator.
  MachineInstr *SecondLastInst = I;

  // If there are three terminators, we don't know what sort of block this is.
  if (SecondLastInst && I != MBB.begin() &&
      isUnpredicatedTerminator(--I))
    return true;

  // If the block ends with Mapip::BR and Mapip::COND_BRANCH_*, handle it.
  if ((SecondLastInst->getOpcode() == Mapip::COND_BRANCH_I ||
      SecondLastInst->getOpcode() == Mapip::COND_BRANCH_F) &&
      LastInst->getOpcode() == Mapip::BR) {
    TBB =  SecondLastInst->getOperand(2).getMBB();
    Cond.push_back(SecondLastInst->getOperand(0));
    Cond.push_back(SecondLastInst->getOperand(1));
    FBB = LastInst->getOperand(0).getMBB();
    return false;
  }

  // If the block ends with two Mapip::BRs, handle it.  The second one is not
  // executed, so remove it.
  if (SecondLastInst->getOpcode() == Mapip::BR &&
      LastInst->getOpcode() == Mapip::BR) {
    TBB = SecondLastInst->getOperand(0).getMBB();
    I = LastInst;
    if (AllowModify)
      I->eraseFromParent();
    return false;
  }

  // Otherwise, can't handle this.
  return true;
}

unsigned MapipInstrInfo::RemoveBranch(MachineBasicBlock &MBB) const {
  MachineBasicBlock::iterator I = MBB.end();
  if (I == MBB.begin()) return 0;
  --I;
  while (I->isDebugValue()) {
    if (I == MBB.begin())
      return 0;
    --I;
  }
  if (I->getOpcode() != Mapip::BR &&
      I->getOpcode() != Mapip::COND_BRANCH_I &&
      I->getOpcode() != Mapip::COND_BRANCH_F)
    return 0;

  // Remove the branch.
  I->eraseFromParent();

  I = MBB.end();

  if (I == MBB.begin()) return 1;
  --I;
  if (I->getOpcode() != Mapip::COND_BRANCH_I &&
      I->getOpcode() != Mapip::COND_BRANCH_F)
    return 1;

  // Remove the branch.
  I->eraseFromParent();
  return 2;
}

void MapipInstrInfo::insertNoop(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI) const {
  DebugLoc DL;
  BuildMI(MBB, MI, DL, get(Mapip::BISr), Mapip::R31)
    .addReg(Mapip::R31)
    .addReg(Mapip::R31);
}

bool MapipInstrInfo::
ReverseBranchCondition(SmallVectorImpl<MachineOperand> &Cond) const {
  assert(Cond.size() == 2 && "Invalid Mapip branch opcode!");
  Cond[0].setImm(MapipRevCondCode(Cond[0].getImm()));
  return false;
}

/// getGlobalBaseReg - Return a virtual register initialized with the
/// the global base register value. Output instructions required to
/// initialize the register in the function entry block, if necessary.
///
unsigned MapipInstrInfo::getGlobalBaseReg(MachineFunction *MF) const {
  MapipMachineFunctionInfo *MapipFI = MF->getInfo<MapipMachineFunctionInfo>();
  unsigned GlobalBaseReg = MapipFI->getGlobalBaseReg();
  if (GlobalBaseReg != 0)
    return GlobalBaseReg;

  // Insert the set of GlobalBaseReg into the first MBB of the function
  MachineBasicBlock &FirstMBB = MF->front();
  MachineBasicBlock::iterator MBBI = FirstMBB.begin();
  MachineRegisterInfo &RegInfo = MF->getRegInfo();
  const TargetInstrInfo *TII = MF->getTarget().getInstrInfo();

  GlobalBaseReg = RegInfo.createVirtualRegister(&Mapip::GPRCRegClass);
  BuildMI(FirstMBB, MBBI, DebugLoc(), TII->get(TargetOpcode::COPY),
          GlobalBaseReg).addReg(Mapip::R29);
  RegInfo.addLiveIn(Mapip::R29);

  MapipFI->setGlobalBaseReg(GlobalBaseReg);
  return GlobalBaseReg;
}

/// getGlobalRetAddr - Return a virtual register initialized with the
/// the global base register value. Output instructions required to
/// initialize the register in the function entry block, if necessary.
///
unsigned MapipInstrInfo::getGlobalRetAddr(MachineFunction *MF) const {
  MapipMachineFunctionInfo *MapipFI = MF->getInfo<MapipMachineFunctionInfo>();
  unsigned GlobalRetAddr = MapipFI->getGlobalRetAddr();
  if (GlobalRetAddr != 0)
    return GlobalRetAddr;

  // Insert the set of GlobalRetAddr into the first MBB of the function
  MachineBasicBlock &FirstMBB = MF->front();
  MachineBasicBlock::iterator MBBI = FirstMBB.begin();
  MachineRegisterInfo &RegInfo = MF->getRegInfo();
  const TargetInstrInfo *TII = MF->getTarget().getInstrInfo();

  GlobalRetAddr = RegInfo.createVirtualRegister(&Mapip::GPRCRegClass);
  BuildMI(FirstMBB, MBBI, DebugLoc(), TII->get(TargetOpcode::COPY),
          GlobalRetAddr).addReg(Mapip::R26);
  RegInfo.addLiveIn(Mapip::R26);

  MapipFI->setGlobalRetAddr(GlobalRetAddr);
  return GlobalRetAddr;
}
