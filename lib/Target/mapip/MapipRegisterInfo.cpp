//===- MapipRegisterInfo.cpp - Mapip Register Information -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Mapip implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "reginfo"
#include "Mapip.h"
#include "MapipRegisterInfo.h"
#include "llvm/Constants.h"
#include "llvm/Type.h"
#include "llvm/Function.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/STLExtras.h"
#include <cstdlib>

#define GET_REGINFO_TARGET_DESC
#include "MapipGenRegisterInfo.inc"

using namespace llvm;

MapipRegisterInfo::MapipRegisterInfo(const TargetInstrInfo &tii)
  : MapipGenRegisterInfo(Mapip::R26), TII(tii) {
}

static long getUpper16(long l) {
  long y = l / Mapip::IMM_MULT;
  if (l % Mapip::IMM_MULT > Mapip::IMM_HIGH)
    ++y;
  return y;
}

static long getLower16(long l) {
  long h = getUpper16(l);
  return l - h * Mapip::IMM_MULT;
}

const unsigned* MapipRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF)
                                                                         const {
  static const unsigned CalleeSavedRegs[] = {
    Mapip::R9, Mapip::R10,
    Mapip::R11, Mapip::R12,
    Mapip::R13, Mapip::R14,
    Mapip::F2, Mapip::F3,
    Mapip::F4, Mapip::F5,
    Mapip::F6, Mapip::F7,
    Mapip::F8, Mapip::F9,  0
  };
  return CalleeSavedRegs;
}

BitVector MapipRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  Reserved.set(Mapip::R15);
  Reserved.set(Mapip::R29);
  Reserved.set(Mapip::R30);
  Reserved.set(Mapip::R31);
  return Reserved;
}

//===----------------------------------------------------------------------===//
// Stack Frame Processing methods
//===----------------------------------------------------------------------===//

void MapipRegisterInfo::
eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator I) const {
  const TargetFrameLowering *TFI = MF.getTarget().getFrameLowering();

  if (TFI->hasFP(MF)) {
    // If we have a frame pointer, turn the adjcallstackup instruction into a
    // 'sub ESP, <amt>' and the adjcallstackdown instruction into 'add ESP,
    // <amt>'
    MachineInstr *Old = I;
    uint64_t Amount = Old->getOperand(0).getImm();
    if (Amount != 0) {
      // We need to keep the stack aligned properly.  To do this, we round the
      // amount of space needed for the outgoing arguments up to the next
      // alignment boundary.
      unsigned Align = TFI->getStackAlignment();
      Amount = (Amount+Align-1)/Align*Align;

      MachineInstr *New;
      if (Old->getOpcode() == Mapip::ADJUSTSTACKDOWN) {
        New=BuildMI(MF, Old->getDebugLoc(), TII.get(Mapip::LDA), Mapip::R30)
          .addImm(-Amount).addReg(Mapip::R30);
      } else {
         assert(Old->getOpcode() == Mapip::ADJUSTSTACKUP);
         New=BuildMI(MF, Old->getDebugLoc(), TII.get(Mapip::LDA), Mapip::R30)
          .addImm(Amount).addReg(Mapip::R30);
      }

      // Replace the pseudo instruction with a new instruction...
      MBB.insert(I, New);
    }
  }

  MBB.erase(I);
}

//Mapip has a slightly funny stack:
//Args
//<- incoming SP
//fixed locals (and spills, callee saved, etc)
//<- FP
//variable locals
//<- SP

void
MapipRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                       int SPAdj, RegScavenger *RS) const {
  assert(SPAdj == 0 && "Unexpected");

  unsigned i = 0;
  MachineInstr &MI = *II;
  MachineBasicBlock &MBB = *MI.getParent();
  MachineFunction &MF = *MBB.getParent();
  const TargetFrameLowering *TFI = MF.getTarget().getFrameLowering();

  bool FP = TFI->hasFP(MF);

  while (!MI.getOperand(i).isFI()) {
    ++i;
    assert(i < MI.getNumOperands() && "Instr doesn't have FrameIndex operand!");
  }

  int FrameIndex = MI.getOperand(i).getIndex();

  // Add the base register of R30 (SP) or R15 (FP).
  MI.getOperand(i + 1).ChangeToRegister(FP ? Mapip::R15 : Mapip::R30, false);

  // Now add the frame object offset to the offset from the virtual frame index.
  int Offset = MF.getFrameInfo()->getObjectOffset(FrameIndex);

  DEBUG(errs() << "FI: " << FrameIndex << " Offset: " << Offset << "\n");

  Offset += MF.getFrameInfo()->getStackSize();

  DEBUG(errs() << "Corrected Offset " << Offset
       << " for stack size: " << MF.getFrameInfo()->getStackSize() << "\n");

  if (Offset > Mapip::IMM_HIGH || Offset < Mapip::IMM_LOW) {
    DEBUG(errs() << "Unconditionally using R28 for evil purposes Offset: "
          << Offset << "\n");
    //so in this case, we need to use a temporary register, and move the
    //original inst off the SP/FP
    //fix up the old:
    MI.getOperand(i + 1).ChangeToRegister(Mapip::R28, false);
    MI.getOperand(i).ChangeToImmediate(getLower16(Offset));
    //insert the new
    MachineInstr* nMI=BuildMI(MF, MI.getDebugLoc(),
                              TII.get(Mapip::LDAH), Mapip::R28)
      .addImm(getUpper16(Offset)).addReg(FP ? Mapip::R15 : Mapip::R30);
    MBB.insert(II, nMI);
  } else {
    MI.getOperand(i).ChangeToImmediate(Offset);
  }
}

unsigned MapipRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  const TargetFrameLowering *TFI = MF.getTarget().getFrameLowering();

  return TFI->hasFP(MF) ? Mapip::R15 : Mapip::R30;
}

unsigned MapipRegisterInfo::getEHExceptionRegister() const {
  llvm_unreachable("What is the exception register");
  return 0;
}

unsigned MapipRegisterInfo::getEHHandlerRegister() const {
  llvm_unreachable("What is the exception handler register");
  return 0;
}

std::string MapipRegisterInfo::getPrettyName(unsigned reg)
{
  std::string s(MapipRegDesc[reg].Name);
  return s;
}
