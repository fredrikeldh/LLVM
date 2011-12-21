//===- MapipSubtarget.cpp - Mapip Subtarget Information ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Mapip specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "MapipSubtarget.h"
#include "Mapip.h"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "MapipGenSubtargetInfo.inc"

using namespace llvm;

MapipSubtarget::MapipSubtarget(const std::string &TT, const std::string &CPU,
                               const std::string &FS)
  : MapipGenSubtargetInfo(TT, CPU, FS), HasCT(false) {
  std::string CPUName = CPU;
  if (CPUName.empty())
    CPUName = "generic";

  // Parse features string.
  ParseSubtargetFeatures(CPUName, FS);

  // Initialize scheduling itinerary for the specified CPU.
  InstrItins = getInstrItineraryForCPU(CPUName);
}
