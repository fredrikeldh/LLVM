//===- MapipInstrInfo.cpp - Mapip Instruction Information ---------------------===//
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
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

#include "MapipGenInstrInfo.inc"

MapipInstrInfo::MapipInstrInfo(MapipTargetMachine &_TM)
  : TargetInstrInfoImpl(MapipInsts, array_lengthof(MapipInsts)),
    RI(_TM, *this), TM(_TM) {}

static const struct map_entry {
  const TargetRegisterClass *cls;
  const int opcode;
} map[] = {
  { &Mapip::RRegu16RegClass, Mapip::MOVU16rr },
  { &Mapip::RRegu32RegClass, Mapip::MOVU32rr },
  { &Mapip::RRegu64RegClass, Mapip::MOVU64rr },
  { &Mapip::RRegf32RegClass, Mapip::MOVF32rr },
  { &Mapip::RRegf64RegClass, Mapip::MOVF64rr },
  { &Mapip::PredsRegClass,   Mapip::MOVPREDrr }
};

void MapipInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                               MachineBasicBlock::iterator I, DebugLoc DL,
                               unsigned DstReg, unsigned SrcReg,
                               bool KillSrc) const {
  for (int i = 0, e = sizeof(map)/sizeof(map[0]); i != e; ++ i) {
    if (map[i].cls->contains(DstReg, SrcReg)) {
      BuildMI(MBB, I, DL,
              get(map[i].opcode), DstReg).addReg(SrcReg,
                                                 getKillRegState(KillSrc));
      return;
    }
  }

  llvm_unreachable("Impossible reg-to-reg copy");
}

bool MapipInstrInfo::copyRegToReg(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator I,
                                unsigned DstReg, unsigned SrcReg,
                                const TargetRegisterClass *DstRC,
                                const TargetRegisterClass *SrcRC,
                                DebugLoc DL) const {
  if (DstRC != SrcRC)
    return false;

  for (int i = 0, e = sizeof(map)/sizeof(map[0]); i != e; ++ i)
    if (DstRC == map[i].cls) {
      MachineInstr *MI = BuildMI(MBB, I, DL, get(map[i].opcode),
                                 DstReg).addReg(SrcReg);
      if (MI->findFirstPredOperandIdx() == -1) {
        MI->addOperand(MachineOperand::CreateReg(0, false));
        MI->addOperand(MachineOperand::CreateImm(/*IsInv=*/0));
      }
      return true;
    }

  return false;
}

bool MapipInstrInfo::isMoveInstr(const MachineInstr& MI,
                               unsigned &SrcReg, unsigned &DstReg,
                               unsigned &SrcSubIdx, unsigned &DstSubIdx) const {
  switch (MI.getOpcode()) {
    default:
      return false;
    case Mapip::MOVU16rr:
    case Mapip::MOVU32rr:
    case Mapip::MOVU64rr:
    case Mapip::MOVF32rr:
    case Mapip::MOVF64rr:
    case Mapip::MOVPREDrr:
      assert(MI.getNumOperands() >= 2 &&
             MI.getOperand(0).isReg() && MI.getOperand(1).isReg() &&
             "Invalid register-register move instruction");
      SrcSubIdx = DstSubIdx = 0; // No sub-registers
      DstReg = MI.getOperand(0).getReg();
      SrcReg = MI.getOperand(1).getReg();
      return true;
  }
}
