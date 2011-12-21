//===-- MapipBranchSelector.cpp - Convert Pseudo branchs ----------*- C++ -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Replace Pseudo COND_BRANCH_* with their appropriate real branch
// Simplified version of the PPC Branch Selector
//
//===----------------------------------------------------------------------===//

#include "Mapip.h"
#include "MapipInstrInfo.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/MC/MCAsmInfo.h"
using namespace llvm;

namespace {
  struct MapipBSel : public MachineFunctionPass {
    static char ID;
    MapipBSel() : MachineFunctionPass(ID) {}

    virtual bool runOnMachineFunction(MachineFunction &Fn);

    virtual const char *getPassName() const {
      return "Mapip Branch Selection";
    }
  };
  char MapipBSel::ID = 0;
}

/// createMapipBranchSelectionPass - returns an instance of the Branch Selection
/// Pass
///
FunctionPass *llvm::createMapipBranchSelectionPass() {
  return new MapipBSel();
}

bool MapipBSel::runOnMachineFunction(MachineFunction &Fn) {

  for (MachineFunction::iterator MFI = Fn.begin(), E = Fn.end(); MFI != E;
       ++MFI) {
    MachineBasicBlock *MBB = MFI;

    for (MachineBasicBlock::iterator MBBI = MBB->begin(), EE = MBB->end();
         MBBI != EE; ++MBBI) {
      if (MBBI->getOpcode() == Mapip::COND_BRANCH_I ||
          MBBI->getOpcode() == Mapip::COND_BRANCH_F) {

        // condbranch operands:
        // 0. bc opcode
        // 1. reg
        // 2. target MBB
        const TargetInstrInfo *TII = Fn.getTarget().getInstrInfo();
        MBBI->setDesc(TII->get(MBBI->getOperand(0).getImm()));
      }
    }
  }

  return true;
}

