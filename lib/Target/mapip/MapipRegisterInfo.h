//===- MapipRegisterInfo.h - Mapip Register Information Impl ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Mapip implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef MAPIPREGISTERINFO_H
#define MAPIPREGISTERINFO_H

#include "Mapip.h"
#include "llvm/Target/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "MapipGenRegisterInfo.inc"

namespace llvm {
class MapipSubtarget;
class TargetInstrInfo;
class Type;

struct MapipRegisterInfo : public MapipGenRegisterInfo {
	const MapipSubtarget &Subtarget;
	const TargetInstrInfo &TII;

	MapipRegisterInfo(const MapipSubtarget &Subtarget, const TargetInstrInfo &tii);

	/// getRegisterNumbering - Given the enum value for some register, e.g.
	/// Mapip::RA, return the number that it corresponds to (e.g. 31).
	static unsigned getRegisterNumbering(unsigned RegEnum);

	/// Get PIC indirect call register
	static unsigned getPICCallReg();

	/// Adjust the Mapip stack frame.
	void adjustMapipStackFrame(MachineFunction &MF) const;

	/// Code Generation virtual methods...
	const unsigned *getCalleeSavedRegs(const MachineFunction* MF = 0) const;

	BitVector getReservedRegs(const MachineFunction &MF) const;

	void eliminateCallFramePseudoInstr(MachineFunction &MF,
	MachineBasicBlock &MBB,
	MachineBasicBlock::iterator I) const;

	/// Stack Frame Processing Methods
	void eliminateFrameIndex(MachineBasicBlock::iterator II,
		int SPAdj, RegScavenger *RS = NULL) const;

	void processFunctionBeforeFrameFinalized(MachineFunction &MF) const;

	/// Debug information queries.
	unsigned getFrameRegister(const MachineFunction &MF) const;

	/// Exception handling queries.
	unsigned getEHExceptionRegister() const;
	unsigned getEHHandlerRegister() const;
};

} // end namespace llvm

#endif
