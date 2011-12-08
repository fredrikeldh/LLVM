//=======- PTXFrameLowering.cpp - PTX Frame Information -------*- C++ -*-=====//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the PTX implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#include "MapipFrameLowering.h"
#include "llvm/CodeGen/MachineFunction.h"

using namespace llvm;

void MapipFrameLowering::emitPrologue(MachineFunction &MF) const {
}

void MapipFrameLowering::emitEpilogue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
}
