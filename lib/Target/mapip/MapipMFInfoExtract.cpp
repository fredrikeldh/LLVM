//===-- MapipMFInfoExtract.cpp - Extract Mapip machine function info ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines an information extractor for Mapip machine functions.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "ptx-mf-info-extract"

#include "Mapip.h"
#include "MapipTargetMachine.h"
#include "MapipMachineFunctionInfo.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

// NOTE: MapipMFInfoExtract must after register allocation!

namespace llvm {
  /// MapipMFInfoExtract - Mapip specific code to extract of Mapip machine
  /// function information for MapipAsmPrinter
  ///
  class MapipMFInfoExtract : public MachineFunctionPass {
    private:
      static char ID;

    public:
      MapipMFInfoExtract(MapipTargetMachine &TM, CodeGenOpt::Level OptLevel)
        : MachineFunctionPass(ID) {}

      virtual bool runOnMachineFunction(MachineFunction &MF);

      virtual const char *getPassName() const {
        return "Mapip Machine Function Info Extractor";
      }
  }; // class MapipMFInfoExtract
} // namespace llvm

using namespace llvm;

char MapipMFInfoExtract::ID = 0;

bool MapipMFInfoExtract::runOnMachineFunction(MachineFunction &MF) {
  MapipMachineFunctionInfo *MFI = MF.getInfo<MapipMachineFunctionInfo>();
  MachineRegisterInfo &MRI = MF.getRegInfo();

  DEBUG(dbgs() << "******** Mapip FUNCTION LOCAL VAR REG DEF ********\n");

  unsigned retreg = MFI->retReg();

  DEBUG(dbgs()
        << "Mapip::NoRegister == " << Mapip::NoRegister << "\n"
        << "Mapip::NUM_TARGET_REGS == " << Mapip::NUM_TARGET_REGS << "\n");

  DEBUG(for (unsigned reg = Mapip::NoRegister + 1;
             reg < Mapip::NUM_TARGET_REGS; ++reg)
          if (MRI.isPhysRegUsed(reg))
            dbgs() << "Used Reg: " << reg << "\n";);

  // FIXME: This is a slow linear scanning
  for (unsigned reg = Mapip::NoRegister + 1; reg < Mapip::NUM_TARGET_REGS; ++reg)
    if (MRI.isPhysRegUsed(reg) &&
        reg != retreg &&
        (MFI->isKernel() || !MFI->isArgReg(reg)))
      MFI->addLocalVarReg(reg);

  // Notify MachineFunctionInfo that I've done adding local var reg
  MFI->doneAddLocalVar();

  DEBUG(dbgs() << "Return Reg: " << retreg << "\n");

  DEBUG(for (MapipMachineFunctionInfo::reg_iterator
             i = MFI->argRegBegin(), e = MFI->argRegEnd();
             i != e; ++i)
        dbgs() << "Arg Reg: " << *i << "\n";);

  DEBUG(for (MapipMachineFunctionInfo::reg_iterator
             i = MFI->localVarRegBegin(), e = MFI->localVarRegEnd();
             i != e; ++i)
        dbgs() << "Local Var Reg: " << *i << "\n";);

  return false;
}

FunctionPass *llvm::createMapipMFInfoExtract(MapipTargetMachine &TM,
                                           CodeGenOpt::Level OptLevel) {
  return new MapipMFInfoExtract(TM, OptLevel);
}
