//===- Mapip.td - Describe the Mapip Target Machine -----------*- tblgen -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// This is the top level entry point for the Mapip target.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Target-independent interfaces
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// Subtarget Features.
//===----------------------------------------------------------------------===//

//===- Architectural Features ---------------------------------------------===//

// todo
//def FeatureDouble : SubtargetFeature<"double", "SupportsDouble", "true",
//	"Do not demote .f64 to .f32">;

//===----------------------------------------------------------------------===//
// Mapip supported processors.
//===----------------------------------------------------------------------===//

class Proc<string Name, list<SubtargetFeature> Features>
	: Processor<Name, NoItineraries, Features>;

def : Proc<"generic", []>;

//===----------------------------------------------------------------------===//
// Instruction Descriptions
//===----------------------------------------------------------------------===//

include "MapipInstrInfo.td"

def MapipInstrInfo : InstrInfo;

//===----------------------------------------------------------------------===//
// Target Declaration
//===----------------------------------------------------------------------===//

def Mapip : Target {
	let InstructionSet = MapipInstrInfo;
}
