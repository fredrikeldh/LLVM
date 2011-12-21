//===- MapipRegisterInfo.h - Mapip Register Information Impl ----*- C++ -*-===//
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

#ifndef MAPIPREGISTERINFO_H
#define MAPIPREGISTERINFO_H

#include "llvm/Target/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "MapipGenRegisterInfo.inc"

namespace llvm {

class TargetInstrInfo;
class Type;

struct MapipRegisterInfo : public MapipGenRegisterInfo {
  const TargetInstrInfo &TII;

  MapipRegisterInfo(const TargetInstrInfo &tii);

  /// Code Generation virtual methods...
  const unsigned *getCalleeSavedRegs(const MachineFunction *MF = 0) const;

  BitVector getReservedRegs(const MachineFunction &MF) const;

  void eliminateCallFramePseudoInstr(MachineFunction &MF,
                                     MachineBasicBlock &MBB,
                                     MachineBasicBlock::iterator I) const;

  void eliminateFrameIndex(MachineBasicBlock::iterator II,
                           int SPAdj, RegScavenger *RS = NULL) const;

  // Debug information queries.
  unsigned getFrameRegister(const MachineFunction &MF) const;

  // Exception handling queries.
  unsigned getEHExceptionRegister() const;
  unsigned getEHHandlerRegister() const;

  static std::string getPrettyName(unsigned reg);
};

} // end namespace llvm

#endif
