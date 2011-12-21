//==-- MapipFrameLowering.h - Define frame lowering for Mapip --*- C++ -*---==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//
//===----------------------------------------------------------------------===//

#ifndef MAPIP_FRAMEINFO_H
#define MAPIP_FRAMEINFO_H

#include "Mapip.h"
#include "MapipSubtarget.h"
#include "llvm/Target/TargetFrameLowering.h"

namespace llvm {
  class MapipSubtarget;

class MapipFrameLowering : public TargetFrameLowering {
  const MapipSubtarget &STI;
  // FIXME: This should end in MachineFunctionInfo, not here!
  mutable int curgpdist;
public:
  explicit MapipFrameLowering(const MapipSubtarget &sti)
    : TargetFrameLowering(StackGrowsDown, 16, 0), STI(sti), curgpdist(0) {
  }

  /// emitProlog/emitEpilog - These methods insert prolog and epilog code into
  /// the function.
  void emitPrologue(MachineFunction &MF) const;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const;

  bool hasFP(const MachineFunction &MF) const;
};

} // End llvm namespace

#endif
