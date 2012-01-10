//====----------- MapipSubtarget.h - Define Subtarget ----------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef MAPIP_SUBTARGET_H
#define MAPIP_SUBTARGET_H

#include "llvm/Target/TargetSubtargetInfo.h"

#define GET_SUBTARGETINFO_HEADER
#include "MapipGenSubtargetInfo.inc"

namespace llvm {
  class MapipSubtarget : public MapipGenSubtargetInfo {
    private:

      // The native .f64 type is supported on the hardware.
      bool SupportsDouble;

    public:
      MapipSubtarget(const std::string &TT, const std::string &FS);

      std::string getTargetString() const;

      bool supportsDouble() const { return SupportsDouble; }

      //bool use64BitAddresses() const { return Use64BitAddresses; }

      std::string ParseSubtargetFeatures(const std::string &FS,
                                         const std::string &CPU);
  }; // class MapipSubtarget
} // namespace llvm

#endif // MAPIP_SUBTARGET_H
