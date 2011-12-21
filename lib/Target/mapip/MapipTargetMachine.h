//===-- MapipTargetMachine.h - Define TargetMachine for Mapip ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the Mapip-specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef MAPIP_TARGETMACHINE_H
#define MAPIP_TARGETMACHINE_H

#include "MapipInstrInfo.h"
#include "MapipISelLowering.h"
#include "MapipFrameLowering.h"
#include "MapipSelectionDAGInfo.h"
#include "MapipSubtarget.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetFrameLowering.h"

namespace llvm {

class GlobalValue;

class MapipTargetMachine : public LLVMTargetMachine {
  const TargetData DataLayout;       // Calculates type size & alignment
  MapipInstrInfo InstrInfo;
  MapipFrameLowering FrameLowering;
  MapipSubtarget Subtarget;
  MapipTargetLowering TLInfo;
  MapipSelectionDAGInfo TSInfo;

public:
  MapipTargetMachine(const Target &T, StringRef TT,
                     StringRef CPU, StringRef FS,
                     Reloc::Model RM, CodeModel::Model CM);

  virtual const MapipInstrInfo *getInstrInfo() const { return &InstrInfo; }
  virtual const TargetFrameLowering  *getFrameLowering() const {
    return &FrameLowering;
  }
  virtual const MapipSubtarget   *getSubtargetImpl() const{ return &Subtarget; }
  virtual const MapipRegisterInfo *getRegisterInfo() const {
    return &InstrInfo.getRegisterInfo();
  }
  virtual const MapipTargetLowering* getTargetLowering() const {
    return &TLInfo;
  }
  virtual const MapipSelectionDAGInfo* getSelectionDAGInfo() const {
    return &TSInfo;
  }
  virtual const TargetData       *getTargetData() const { return &DataLayout; }

  // Pass Pipeline Configuration
  virtual bool addInstSelector(PassManagerBase &PM, CodeGenOpt::Level OptLevel);
  virtual bool addPreEmitPass(PassManagerBase &PM, CodeGenOpt::Level OptLevel);
};

} // end namespace llvm

#endif
