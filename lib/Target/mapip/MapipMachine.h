//==-- MapipTargetMachine.h - TargetMachine for the C++ backend --*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the TargetMachine that is used by the Mapip backend.
//
//===----------------------------------------------------------------------===//

#ifndef MAPIP_MACHINE_H
#define MAPIP_MACHINE_H

#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetData.h"

namespace llvm {

class formatted_raw_ostream;

struct MapipMachine : public LLVMTargetMachine {
	MapipMachine(const Target& T, StringRef TT,
		StringRef CPU, StringRef FS,
		Reloc::Model RM, CodeModel::Model CM);

	virtual const TargetData* getTargetData() const { return 0; }

	virtual void dummy();
};

extern Target TheMapipBackendTarget;

} // End llvm namespace

#endif	//MAPIP_MACHINE_H
