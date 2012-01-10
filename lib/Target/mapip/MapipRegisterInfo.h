//===- MapipRegisterInfo.h - Mapip Register Information Impl --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Mapip implementation of the MRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef Mapip_REGISTER_INFO_H
#define Mapip_REGISTER_INFO_H

#include "llvm/Support/ErrorHandling.h"
#include "llvm/ADT/BitVector.h"

#define GET_REGINFO_HEADER
#include "MapipGenRegisterInfo.inc"

namespace llvm {
class MapipTargetMachine;
class MachineFunction;

struct MapipRegisterInfo : public MapipGenRegisterInfo {
  MapipRegisterInfo(MapipTargetMachine &TM,
                  const TargetInstrInfo &TII) : MapipGenRegisterInfo(32) {}

  virtual const unsigned
    *getCalleeSavedRegs(const MachineFunction *MF = 0) const {
    static const unsigned CalleeSavedRegs[] = { 0 };
    return CalleeSavedRegs; // save nothing
  }

  virtual BitVector getReservedRegs(const MachineFunction &MF) const {
    BitVector Reserved(getNumRegs());
    return Reserved; // reserve no regs
  }

  virtual void eliminateFrameIndex(MachineBasicBlock::iterator MI,
                                   int SPAdj,
                                   RegScavenger *RS = NULL) const {
    llvm_unreachable("Mapip does not support general function call");
  }

  virtual unsigned getFrameRegister(const MachineFunction &MF) const {
    llvm_unreachable("Mapip does not have a frame register");
    return 0;
  }

  virtual unsigned getRARegister() const {
    llvm_unreachable("Mapip does not have a return address register");
    return 0;
  }

  /*virtual int getDwarfRegNum(unsigned RegNum, bool isEH) const {
    return MapipGenRegisterInfo::getDwarfRegNumFull(RegNum, 0);
  }*/
}; // struct MapipRegisterInfo
} // namespace llvm

#endif // Mapip_REGISTER_INFO_H
