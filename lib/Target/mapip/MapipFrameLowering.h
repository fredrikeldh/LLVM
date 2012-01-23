//==--- MapipFrameLowering.h - Define frame lowering for Mapip --*- C++ -*---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//
//===----------------------------------------------------------------------===//

#ifndef MAPIP_FRAMEINFO_H
#define MAPIP_FRAMEINFO_H

#include "Mapip.h"
#include "MapipSubtarget.h"
#include "llvm/Target/TargetFrameLowering.h"

namespace llvm {
class MapipSubtarget;

class MapipFrameLowering : public TargetFrameLowering {
protected:
	const MapipSubtarget &STI;

public:
	explicit MapipFrameLowering(const MapipSubtarget &sti)
		: TargetFrameLowering(StackGrowsDown, 8, 0),
		STI(sti) {
	}

	bool targetHandlesStackFrameRounding() const;

	/// emitProlog/emitEpilog - These methods insert prolog and epilog code into
	/// the function.
	void emitPrologue(MachineFunction &MF) const;
	void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const;

	bool hasFP(const MachineFunction &MF) const;

	void processFunctionBeforeCalleeSavedScan(MachineFunction &MF,
		RegScavenger *RS) const;
};

} // End llvm namespace

#endif
