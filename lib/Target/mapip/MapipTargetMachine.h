//===-- MapipTargetMachine.h - Define TargetMachine for Mapip -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the Mapip specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef Mapip_TARGET_MACHINE_H
#define Mapip_TARGET_MACHINE_H

#include "MapipISelLowering.h"
#include "MapipInstrInfo.h"
#include "MapipFrameLowering.h"
#include "MapipSubtarget.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class MapipTargetMachine : public LLVMTargetMachine {
  private:
    const TargetData  DataLayout;
    MapipSubtarget      Subtarget; // has to be initialized before FrameLowering
    MapipFrameLowering  FrameLowering;
    MapipInstrInfo      InstrInfo;
    MapipTargetLowering TLInfo;

  public:
    MapipTargetMachine(const Target &T, const std::string &TT,
                     const std::string &FS);

    virtual const TargetData *getTargetData() const { return &DataLayout; }

    virtual const TargetFrameLowering *getFrameLowering() const {
      return &FrameLowering;
    }

    virtual const MapipInstrInfo *getInstrInfo() const { return &InstrInfo; }
    virtual const TargetRegisterInfo *getRegisterInfo() const {
      return &InstrInfo.getRegisterInfo(); }

    virtual const MapipTargetLowering *getTargetLowering() const {
      return &TLInfo; }

    virtual const MapipSubtarget *getSubtargetImpl() const { return &Subtarget; }

    virtual bool addInstSelector(PassManagerBase &PM,
                                 CodeGenOpt::Level OptLevel);
    virtual bool addPostRegAlloc(PassManagerBase &PM,
                                 CodeGenOpt::Level OptLevel);
}; // class MapipTargetMachine
} // namespace llvm

#endif // Mapip_TARGET_MACHINE_H
