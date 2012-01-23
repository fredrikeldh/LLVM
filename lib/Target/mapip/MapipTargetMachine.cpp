//===-- MapipTargetMachine.cpp - Define TargetMachine for Mapip -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Implements the info about Mapip target spec.
//
//===----------------------------------------------------------------------===//

#include "Mapip.h"
#include "MapipTargetMachine.h"
#include "llvm/PassManager.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

extern "C" void LLVMInitializeMapipTarget() {
	// Register the target.
	RegisterTargetMachine<MapipebTargetMachine> X(TheMapipTarget);
}

// DataLayout --> Big-endian, 32-bit pointer/ABI/alignment
// The stack is always 8 byte aligned
// On function prologue, the stack is created by decrementing
// its pointer. Once decremented, all references are done with positive
// offset from the stack/frame pointer, using StackGrowsUp enables
// an easier handling.
// Using CodeModel::Large enables different CALL behavior.
MapipTargetMachine::
	MapipTargetMachine(const Target &T, StringRef TT,
	StringRef CPU, StringRef FS,
	Reloc::Model RM, CodeModel::Model CM,
	bool isLittle):
LLVMTargetMachine(T, TT, CPU, FS, RM, CM),
	Subtarget(TT, CPU, FS, isLittle),
	DataLayout(isLittle ?
		"e-p:32:32:32-i8:8:32-i16:16:32-i64:64:64-n32" :
		"E-p:32:32:32-i8:8:32-i16:16:32-i64:64:64-n32"),
	InstrInfo(*this),
	FrameLowering(Subtarget),
	TLInfo(*this), TSInfo(*this) {
}

MapipebTargetMachine::
	MapipebTargetMachine(const Target &T, StringRef TT,
	StringRef CPU, StringRef FS,
	Reloc::Model RM, CodeModel::Model CM) :
MapipTargetMachine(T, TT, CPU, FS, RM, CM, false) {}

MapipelTargetMachine::
	MapipelTargetMachine(const Target &T, StringRef TT,
	StringRef CPU, StringRef FS,
	Reloc::Model RM, CodeModel::Model CM) :
MapipTargetMachine(T, TT, CPU, FS, RM, CM, true) {}

// Install an instruction selector pass using
// the ISelDag to gen Mapip code.
bool MapipTargetMachine::
	addInstSelector(PassManagerBase &PM, CodeGenOpt::Level OptLevel)
{
	PM.add(createMapipISelDag(*this));
	return false;
}
