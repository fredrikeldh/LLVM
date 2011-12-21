//=====-- MapipSubtarget.h - Define Subtarget for the Mapip --*- C++ -*--====//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the Mapip specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#ifndef MAPIPSUBTARGET_H
#define MAPIPSUBTARGET_H

#include "llvm/Target/TargetSubtargetInfo.h"
#include "llvm/MC/MCInstrItineraries.h"
#include <string>

#define GET_SUBTARGETINFO_HEADER
#include "MapipGenSubtargetInfo.inc"

namespace llvm {
class StringRe;

class MapipSubtarget : public MapipGenSubtargetInfo {
protected:

  bool HasCT;

  InstrItineraryData InstrItins;

public:
  /// This constructor initializes the data members to match that
  /// of the specified triple.
  ///
  MapipSubtarget(const std::string &TT, const std::string &CPU,
                 const std::string &FS);

  /// ParseSubtargetFeatures - Parses features string setting specified
  /// subtarget options.  Definition of function is auto generated by tblgen.
  void ParseSubtargetFeatures(StringRef CPU, StringRef FS);

  bool hasCT() const { return HasCT; }
};
} // End llvm namespace

#endif