//===-- PTXTargetMachine.cpp - Define TargetMachine for PTX ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Top-level implementation for the PTX target.
//
//===----------------------------------------------------------------------===//

#include "Mapip.h"
#include "MapipMCAsmInfo.h"
#include "MapipTargetMachine.h"
#include "llvm/PassManager.h"
#include "llvm/Target/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace llvm {
  MCStreamer *createMapipAsmStreamer(MCContext &Ctx, formatted_raw_ostream &OS,
                                   bool isVerboseAsm, bool useLoc,
                                   MCInstPrinter *InstPrint,
                                   MCCodeEmitter *CE,
                                   TargetAsmBackend *TAB,
                                   bool ShowInst);
}

extern "C" void LLVMInitializeMapipTarget() {
  RegisterTargetMachine<MapipTargetMachine> X(TheMapipTarget);
  RegisterAsmInfo<MapipMCAsmInfo> Y(TheMapipTarget);
  TargetRegistry::RegisterAsmStreamer(TheMapipTarget, createMapipAsmStreamer);
}

namespace {
  const char* DataLayout32 =
    "e-p:32:32";
}

// DataLayout and FrameLowering are filled with dummy data
MapipTargetMachine::MapipTargetMachine(const Target &T,
                                   const std::string &TT,
                                   const std::string &FS)
  : LLVMTargetMachine(T, TT),
    // FIXME: This feels like a dirty hack, but Subtarget does not appear to be
    //        initialized at this point, and we need to finish initialization of
    //        DataLayout.
    DataLayout(DataLayout32),
    Subtarget(TT, FS),
    FrameLowering(Subtarget),
    InstrInfo(*this),
    TLInfo(*this) {
}

bool MapipTargetMachine::addInstSelector(PassManagerBase &PM,
                                       CodeGenOpt::Level OptLevel) {
  PM.add(createMapipISelDag(*this, OptLevel));
  return false;
}

bool MapipTargetMachine::addPostRegAlloc(PassManagerBase &PM,
                                       CodeGenOpt::Level OptLevel) {
  // PTXMFInfoExtract must after register allocation!
  PM.add(createMapipMFInfoExtract(*this, OptLevel));
  return false;
}
