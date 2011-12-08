//===-- PTX.h - Top-level interface for PTX representation ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in the LLVM
// PTX back-end.
//
//===----------------------------------------------------------------------===//

#ifndef MAPIP_H
#define MAPIP_H

#include "llvm/Target/TargetMachine.h"

namespace llvm {
  class MapipTargetMachine;
  class FunctionPass;

  namespace Mapip {
    enum StateSpace {
      GLOBAL = 0, // default to global state space
      CONSTANT = 1,
      LOCAL = 2,
      PARAMETER = 3,
      SHARED = 4
    };
  } // namespace PTX

  FunctionPass *createMapipISelDag(MapipTargetMachine &TM,
                                 CodeGenOpt::Level OptLevel);

  FunctionPass *createMapipMFInfoExtract(MapipTargetMachine &TM,
                                       CodeGenOpt::Level OptLevel);

  extern Target TheMapipTarget;
} // namespace llvm;

// Defines symbolic names for PTX registers.
//#include "MapipGenRegisterNames.inc"

// Defines symbolic names for the PTX instructions.
//#include "MapipGenInstrNames.inc"

#endif // MAPIP_H
