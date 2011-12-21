//===-- MapipTargetInfo.cpp - Mapip Target Implementation -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Mapip.h"
#include "llvm/Module.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

llvm::Target llvm::TheMapipTarget;

extern "C" void LLVMInitializeMapipTargetInfo() {
  RegisterTarget<Triple::mapip, /*HasJIT=*/true>
    X(TheMapipTarget, "mapip", "Mapip [experimental]");
}
