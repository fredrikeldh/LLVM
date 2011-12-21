//=====- MapipFrameLowering.cpp - Mapip Frame Information ------*- C++ -*-====//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Mapip implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#include "MapipFrameLowering.h"
#include "MapipInstrInfo.h"
#include "MapipMachineFunctionInfo.h"
#include "llvm/Function.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/ADT/Twine.h"

using namespace llvm;

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

// hasFP - Return true if the specified function should have a dedicated frame
// pointer register.  This is true if the function has variable sized allocas or
// if frame pointer elimination is disabled.
//
bool MapipFrameLowering::hasFP(const MachineFunction &MF) const {
  const MachineFrameInfo *MFI = MF.getFrameInfo();
  return MFI->hasVarSizedObjects();
}

void MapipFrameLowering::emitPrologue(MachineFunction &MF) const {
  MachineBasicBlock &MBB = MF.front();   // Prolog goes in entry BB
  MachineBasicBlock::iterator MBBI = MBB.begin();
  MachineFrameInfo *MFI = MF.getFrameInfo();
  const TargetInstrInfo &TII = *MF.getTarget().getInstrInfo();

  DebugLoc dl = (MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc());
  bool FP = hasFP(MF);

  // Handle GOP offset
  BuildMI(MBB, MBBI, dl, TII.get(Mapip::LDAHg), Mapip::R29)
    .addGlobalAddress(MF.getFunction()).addReg(Mapip::R27).addImm(++curgpdist);
  BuildMI(MBB, MBBI, dl, TII.get(Mapip::LDAg), Mapip::R29)
    .addGlobalAddress(MF.getFunction()).addReg(Mapip::R29).addImm(curgpdist);

  BuildMI(MBB, MBBI, dl, TII.get(Mapip::ALTENT))
    .addGlobalAddress(MF.getFunction());

  // Get the number of bytes to allocate from the FrameInfo
  long NumBytes = MFI->getStackSize();

  if (FP)
    NumBytes += 8; //reserve space for the old FP

  // Do we need to allocate space on the stack?
  if (NumBytes == 0) return;

  unsigned Align = getStackAlignment();
  NumBytes = (NumBytes+Align-1)/Align*Align;

  // Update frame info to pretend that this is part of the stack...
  MFI->setStackSize(NumBytes);

  // adjust stack pointer: r30 -= numbytes
  NumBytes = -NumBytes;
  if (NumBytes >= Mapip::IMM_LOW) {
    BuildMI(MBB, MBBI, dl, TII.get(Mapip::LDA), Mapip::R30).addImm(NumBytes)
      .addReg(Mapip::R30);
  } else if (getUpper16(NumBytes) >= Mapip::IMM_LOW) {
    BuildMI(MBB, MBBI, dl, TII.get(Mapip::LDAH), Mapip::R30)
      .addImm(getUpper16(NumBytes)).addReg(Mapip::R30);
    BuildMI(MBB, MBBI, dl, TII.get(Mapip::LDA), Mapip::R30)
      .addImm(getLower16(NumBytes)).addReg(Mapip::R30);
  } else {
    report_fatal_error("Too big a stack frame at " + Twine(NumBytes));
  }

  // Now if we need to, save the old FP and set the new
  if (FP) {
    BuildMI(MBB, MBBI, dl, TII.get(Mapip::STQ))
      .addReg(Mapip::R15).addImm(0).addReg(Mapip::R30);
    // This must be the last instr in the prolog
    BuildMI(MBB, MBBI, dl, TII.get(Mapip::BISr), Mapip::R15)
      .addReg(Mapip::R30).addReg(Mapip::R30);
  }

}

void MapipFrameLowering::emitEpilogue(MachineFunction &MF,
                                  MachineBasicBlock &MBB) const {
  const MachineFrameInfo *MFI = MF.getFrameInfo();
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  const TargetInstrInfo &TII = *MF.getTarget().getInstrInfo();

  assert((MBBI->getOpcode() == Mapip::RETDAG ||
          MBBI->getOpcode() == Mapip::RETDAGp)
         && "Can only insert epilog into returning blocks");
  DebugLoc dl = MBBI->getDebugLoc();

  bool FP = hasFP(MF);

  // Get the number of bytes allocated from the FrameInfo...
  long NumBytes = MFI->getStackSize();

  //now if we need to, restore the old FP
  if (FP) {
    //copy the FP into the SP (discards allocas)
    BuildMI(MBB, MBBI, dl, TII.get(Mapip::BISr), Mapip::R30).addReg(Mapip::R15)
      .addReg(Mapip::R15);
    //restore the FP
    BuildMI(MBB, MBBI, dl, TII.get(Mapip::LDQ), Mapip::R15)
      .addImm(0).addReg(Mapip::R15);
  }

  if (NumBytes != 0) {
    if (NumBytes <= Mapip::IMM_HIGH) {
      BuildMI(MBB, MBBI, dl, TII.get(Mapip::LDA), Mapip::R30).addImm(NumBytes)
        .addReg(Mapip::R30);
    } else if (getUpper16(NumBytes) <= Mapip::IMM_HIGH) {
      BuildMI(MBB, MBBI, dl, TII.get(Mapip::LDAH), Mapip::R30)
        .addImm(getUpper16(NumBytes)).addReg(Mapip::R30);
      BuildMI(MBB, MBBI, dl, TII.get(Mapip::LDA), Mapip::R30)
        .addImm(getLower16(NumBytes)).addReg(Mapip::R30);
    } else {
      report_fatal_error("Too big a stack frame at " + Twine(NumBytes));
    }
  }
}
