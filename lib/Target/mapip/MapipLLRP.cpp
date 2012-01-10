//===-- MapipLLRP.cpp - Mapip Load Load Replay Trap elimination pass. -- --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Here we check for potential replay traps introduced by the spiller
// We also align some branch targets if we can do so for free.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "mapip-nops"
#include "Mapip.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/ADT/SetOperations.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/CommandLine.h"
using namespace llvm;

STATISTIC(nopintro, "Number of nops inserted");
STATISTIC(nopalign, "Number of nops inserted for alignment");

namespace {

  struct MapipLLRPPass : public MachineFunctionPass {
    /// Target machine description which we query for reg. names, data
    /// layout, etc.
    ///
    MapipTargetMachine &TM;

    static char ID;
    MapipLLRPPass(MapipTargetMachine &tm)
      : MachineFunctionPass(ID), TM(tm) { }

    virtual const char *getPassName() const {
      return "Mapip NOP inserter";
    }

    bool runOnMachineFunction(MachineFunction &F) {
      const TargetInstrInfo *TII = F.getTarget().getInstrInfo();
      bool Changed = false;
      MachineInstr* prev[3] = {0,0,0};
      DebugLoc dl;
      unsigned count = 0;
      for (MachineFunction::iterator FI = F.begin(), FE = F.end();
           FI != FE; ++FI) {
        MachineBasicBlock& MBB = *FI;
        bool ub = false;
        for (MachineBasicBlock::iterator I = MBB.begin(); I != MBB.end(); ) {
          if (count%4 == 0)
            prev[0] = prev[1] = prev[2] = 0; //Slots cleared at fetch boundary
          ++count;
          MachineInstr *MI = I++;
          switch (MI->getOpcode()) {
          case Mapip::LDQ:  case Mapip::LDL:
          case Mapip::LDWU: case Mapip::LDBU:
          case Mapip::LDT: case Mapip::LDS:
          case Mapip::STQ:  case Mapip::STL:
          case Mapip::STW:  case Mapip::STB:
          case Mapip::STT: case Mapip::STS:
           if (MI->getOperand(2).getReg() == Mapip::R30) {
             if (prev[0] &&
                 prev[0]->getOperand(2).getReg() == MI->getOperand(2).getReg()&&
                 prev[0]->getOperand(1).getImm() == MI->getOperand(1).getImm()){
               prev[0] = prev[1];
               prev[1] = prev[2];
               prev[2] = 0;
               BuildMI(MBB, MI, dl, TII->get(Mapip::BISr), Mapip::R31)
                 .addReg(Mapip::R31)
                 .addReg(Mapip::R31);
               Changed = true; nopintro += 1;
               count += 1;
             } else if (prev[1]
                        && prev[1]->getOperand(2).getReg() ==
                        MI->getOperand(2).getReg()
                        && prev[1]->getOperand(1).getImm() ==
                        MI->getOperand(1).getImm()) {
               prev[0] = prev[2];
               prev[1] = prev[2] = 0;
               BuildMI(MBB, MI, dl, TII->get(Mapip::BISr), Mapip::R31)
                 .addReg(Mapip::R31)
                 .addReg(Mapip::R31);
               BuildMI(MBB, MI, dl, TII->get(Mapip::BISr), Mapip::R31)
                 .addReg(Mapip::R31)
                 .addReg(Mapip::R31);
               Changed = true; nopintro += 2;
               count += 2;
             } else if (prev[2]
                        && prev[2]->getOperand(2).getReg() ==
                        MI->getOperand(2).getReg()
                        && prev[2]->getOperand(1).getImm() ==
                        MI->getOperand(1).getImm()) {
               prev[0] = prev[1] = prev[2] = 0;
               BuildMI(MBB, MI, dl, TII->get(Mapip::BISr), Mapip::R31)
                 .addReg(Mapip::R31).addReg(Mapip::R31);
               BuildMI(MBB, MI, dl, TII->get(Mapip::BISr), Mapip::R31)
                 .addReg(Mapip::R31).addReg(Mapip::R31);
               BuildMI(MBB, MI, dl, TII->get(Mapip::BISr), Mapip::R31)
                 .addReg(Mapip::R31).addReg(Mapip::R31);
               Changed = true; nopintro += 3;
               count += 3;
             }
             prev[0] = prev[1];
             prev[1] = prev[2];
             prev[2] = MI;
             break;
           }
           prev[0] = prev[1];
           prev[1] = prev[2];
           prev[2] = 0;
           break;
          case Mapip::ALTENT:
          case Mapip::MEMLABEL:
          case Mapip::PCLABEL:
            --count;
            break;
          case Mapip::BR:
          case Mapip::JMP:
            ub = true;
            //fall through
          default:
            prev[0] = prev[1];
            prev[1] = prev[2];
            prev[2] = 0;
            break;
          }
        }
        if (ub) {
          //we can align stuff for free at this point
          while (count % 4) {
            BuildMI(MBB, MBB.end(), dl, TII->get(Mapip::BISr), Mapip::R31)
              .addReg(Mapip::R31).addReg(Mapip::R31);
            ++count;
            ++nopalign;
            prev[0] = prev[1];
            prev[1] = prev[2];
            prev[2] = 0;
          }
        }
      }
      return Changed;
    }
  };
  char MapipLLRPPass::ID = 0;
} // end of anonymous namespace

FunctionPass *llvm::createMapipLLRPPass(MapipTargetMachine &tm) {
  return new MapipLLRPPass(tm);
}
