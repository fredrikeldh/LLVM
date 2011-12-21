//===- Mapip.td - Describe the Mapip Target Machine --------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

// Get the target-independent interfaces which we are implementing...
//
include "llvm/Target/Target.td"

//Mapip is little endian

//===----------------------------------------------------------------------===//
// Subtarget Features
//===----------------------------------------------------------------------===//

def FeatureCIX : SubtargetFeature<"cix", "HasCT", "true",
                                  "Enable CIX extensions">;

//===----------------------------------------------------------------------===//
// Register File Description
//===----------------------------------------------------------------------===//

include "MapipRegisterInfo.td"

//===----------------------------------------------------------------------===//
// Calling Convention Description
//===----------------------------------------------------------------------===//

include "MapipCallingConv.td"

//===----------------------------------------------------------------------===//
// Schedule Description
//===----------------------------------------------------------------------===//

include "MapipSchedule.td"

//===----------------------------------------------------------------------===//
// Instruction Descriptions
//===----------------------------------------------------------------------===//

include "MapipInstrInfo.td"

def MapipInstrInfo : InstrInfo;

//===----------------------------------------------------------------------===//
// Mapip Processor Definitions
//===----------------------------------------------------------------------===//

def : Processor<"generic", Mapip21264Itineraries, []>;
def : Processor<"ev6"    , Mapip21264Itineraries, []>;
def : Processor<"ev67"   , Mapip21264Itineraries, [FeatureCIX]>;

//===----------------------------------------------------------------------===//
// The Mapip Target
//===----------------------------------------------------------------------===//


def Mapip : Target {
  // Pull in Instruction Info:
  let InstructionSet = MapipInstrInfo;
}
