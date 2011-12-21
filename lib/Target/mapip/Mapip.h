//===-- Mapip.h - Top-level interface for Mapip representation --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in the LLVM
// Mapip back-end.
//
//===----------------------------------------------------------------------===//

#ifndef TARGET_MAPIP_H
#define TARGET_MAPIP_H

#include "MCTargetDesc/MapipMCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
  namespace Mapip {
    // These describe LDAx

    static const int IMM_LOW  = -32768;
    static const int IMM_HIGH = 32767;
    static const int IMM_MULT = 65536;
  }

  class MapipTargetMachine;
  class FunctionPass;
  class formatted_raw_ostream;

  FunctionPass *createMapipISelDag(MapipTargetMachine &TM);
  FunctionPass *createMapipPatternInstructionSelector(TargetMachine &TM);
  FunctionPass *createMapipJITCodeEmitterPass(MapipTargetMachine &TM,
                                              JITCodeEmitter &JCE);
  FunctionPass *createMapipLLRPPass(MapipTargetMachine &tm);
  FunctionPass *createMapipBranchSelectionPass();

} // end namespace llvm;

#endif
