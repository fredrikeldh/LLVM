//====- MapipMachineFuctionInfo.h - Mapip machine function info -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares Mapip-specific per-machine-function information.
//
//===----------------------------------------------------------------------===//

#ifndef MAPIPMACHINEFUNCTIONINFO_H
#define MAPIPMACHINEFUNCTIONINFO_H

#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {

/// MapipMachineFunctionInfo - This class is derived from MachineFunction
/// private Mapip target-specific information for each MachineFunction.
class MapipMachineFunctionInfo : public MachineFunctionInfo {
  /// GlobalBaseReg - keeps track of the virtual register initialized for
  /// use as the global base register. This is used for PIC in some PIC
  /// relocation models.
  unsigned GlobalBaseReg;

  /// GlobalRetAddr = keeps track of the virtual register initialized for
  /// the return address value.
  unsigned GlobalRetAddr;

  /// VarArgsOffset - What is the offset to the first vaarg
  int VarArgsOffset;
  /// VarArgsBase - What is the base FrameIndex
  int VarArgsBase;

public:
  MapipMachineFunctionInfo() : GlobalBaseReg(0), GlobalRetAddr(0),
                               VarArgsOffset(0), VarArgsBase(0) {}

  explicit MapipMachineFunctionInfo(MachineFunction &MF) : GlobalBaseReg(0),
                                                           GlobalRetAddr(0),
                                                           VarArgsOffset(0),
                                                           VarArgsBase(0) {}

  unsigned getGlobalBaseReg() const { return GlobalBaseReg; }
  void setGlobalBaseReg(unsigned Reg) { GlobalBaseReg = Reg; }

  unsigned getGlobalRetAddr() const { return GlobalRetAddr; }
  void setGlobalRetAddr(unsigned Reg) { GlobalRetAddr = Reg; }

  int getVarArgsOffset() const { return VarArgsOffset; }
  void setVarArgsOffset(int Offset) { VarArgsOffset = Offset; }

  int getVarArgsBase() const { return VarArgsBase; }
  void setVarArgsBase(int Base) { VarArgsBase = Base; }
};

} // End llvm namespace

#endif
