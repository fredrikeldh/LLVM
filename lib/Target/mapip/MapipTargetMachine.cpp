//===-- MapipTargetMachine.cpp - Define TargetMachine for Mapip -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

#include "Mapip.h"
#include "MapipTargetMachine.h"
#include "llvm/PassManager.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

extern "C" void LLVMInitializeMapipTarget() {
  // Register the target.
  RegisterTargetMachine<MapipTargetMachine> X(TheMapipTarget);
}

MapipTargetMachine::MapipTargetMachine(const Target &T, StringRef TT,
                                       StringRef CPU, StringRef FS,
                                       Reloc::Model RM, CodeModel::Model CM)
  : LLVMTargetMachine(T, TT, CPU, FS, RM, CM),
    DataLayout("e-f128:128:128-n64"),
    FrameLowering(Subtarget),
    Subtarget(TT, CPU, FS),
    TLInfo(*this),
    TSInfo(*this) {
}

//===----------------------------------------------------------------------===//
// Pass Pipeline Configuration
//===----------------------------------------------------------------------===//

bool MapipTargetMachine::addInstSelector(PassManagerBase &PM,
                                         CodeGenOpt::Level OptLevel) {
  PM.add(createMapipISelDag(*this));
  return false;
}
bool MapipTargetMachine::addPreEmitPass(PassManagerBase &PM,
                                        CodeGenOpt::Level OptLevel) {
  // Must run branch selection immediately preceding the asm printer
  PM.add(createMapipBranchSelectionPass());
  PM.add(createMapipLLRPPass(*this));
  return false;
}
