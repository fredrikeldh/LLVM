//===-- MapipTargetMachine.h - Define TargetMachine for Mapip -00--*- C++ -*-===//
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

#ifndef MAPIPTARGETMACHINE_H
#define MAPIPTARGETMACHINE_H

#include "MapipSubtarget.h"
#include "MapipInstrInfo.h"
#include "MapipISelLowering.h"
#include "MapipFrameLowering.h"
#include "MapipSelectionDAGInfo.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetFrameLowering.h"

namespace llvm {
class formatted_raw_ostream;

class MapipTargetMachine : public LLVMTargetMachine {
	MapipSubtarget Subtarget;
	const TargetData DataLayout; // Calculates type size & alignment
	MapipInstrInfo InstrInfo;
	MapipFrameLowering FrameLowering;
	MapipTargetLowering TLInfo;
	MapipSelectionDAGInfo TSInfo;

public:
	MapipTargetMachine(const Target &T, StringRef TT,
		StringRef CPU, StringRef FS,
		Reloc::Model RM, CodeModel::Model CM,
		bool isLittle);

	virtual const MapipInstrInfo* getInstrInfo() const { return &InstrInfo; }
	virtual const TargetFrameLowering* getFrameLowering() const { return &FrameLowering; }
	virtual const MapipSubtarget* getSubtargetImpl() const { return &Subtarget; }
	virtual const TargetData* getTargetData() const { return &DataLayout;}

	virtual const MapipRegisterInfo *getRegisterInfo()  const {
		return &InstrInfo.getRegisterInfo();
	}

	virtual const MapipTargetLowering *getTargetLowering() const {
		return &TLInfo;
	}

	virtual const MapipSelectionDAGInfo* getSelectionDAGInfo() const {
		return &TSInfo;
	}

	// Pass Pipeline Configuration
	virtual bool addInstSelector(PassManagerBase &PM,
		CodeGenOpt::Level OptLevel);
	virtual bool addPreEmitPass(PassManagerBase &PM,
		CodeGenOpt::Level OptLevel);
	virtual bool addPreRegAlloc(PassManagerBase &PM,
		CodeGenOpt::Level OptLevel);
	virtual bool addPostRegAlloc(PassManagerBase &, CodeGenOpt::Level);
};

/// MapipebTargetMachine - Mapip32 big endian target machine.
///
class MapipebTargetMachine : public MapipTargetMachine {
public:
	MapipebTargetMachine(const Target &T, StringRef TT,
		StringRef CPU, StringRef FS,
		Reloc::Model RM, CodeModel::Model CM);
};

/// MapipelTargetMachine - Mapip32 little endian target machine.
///
class MapipelTargetMachine : public MapipTargetMachine {
public:
	MapipelTargetMachine(const Target &T, StringRef TT,
		StringRef CPU, StringRef FS,
		Reloc::Model RM, CodeModel::Model CM);
};
} // End llvm namespace

#endif
