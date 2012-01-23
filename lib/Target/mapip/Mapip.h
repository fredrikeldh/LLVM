//===-- Mapip.h - Top-level interface for Mapip representation ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in
// the LLVM Mapip back-end.
//
//===----------------------------------------------------------------------===//

#ifndef TARGET_MAPIP_H
#define TARGET_MAPIP_H

#include "MCTargetDesc/MapipMCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
	class MapipTargetMachine;
	class FunctionPass;
	class MachineCodeEmitter;
	class formatted_raw_ostream;

	FunctionPass* createMapipISelDag(MapipTargetMachine &TM);
} // end namespace llvm;

#endif
