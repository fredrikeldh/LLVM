//===- MapipSubtarget.cpp - Mapip Subtarget Information ---------------*- C++ -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Mapip specific subclass of TargetSubtarget.
//
//===----------------------------------------------------------------------===//

#include "MapipSubtarget.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

MapipSubtarget::MapipSubtarget(const std::string &TT, const std::string &FS)
  : MapipShaderModel(Mapip_SM_1_0),
    MapipVersion(Mapip_VERSION_1_4),
    SupportsDouble(false),
    Use64BitAddresses(false) {
  std::string TARGET = "generic";
  ParseSubtargetFeatures(FS, TARGET);
}

std::string MapipSubtarget::getTargetString() const {
  switch(MapipShaderModel) {
    default: llvm_unreachable("Unknown shader model");
    case Mapip_SM_1_0: return "sm_10";
    case Mapip_SM_1_3: return "sm_13";
    case Mapip_SM_2_0: return "sm_20";
  }
}

std::string MapipSubtarget::getMapipVersionString() const {
  switch(MapipVersion) {
    default: llvm_unreachable("Unknown Mapip version");
    case Mapip_VERSION_1_4: return "1.4";
    case Mapip_VERSION_2_0: return "2.0";
    case Mapip_VERSION_2_1: return "2.1";
  }
}

#include "MapipGenSubtarget.inc"
