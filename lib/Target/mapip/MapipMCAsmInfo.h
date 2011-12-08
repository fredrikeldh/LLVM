//=====-- MapipMCAsmInfo.h - Mapip asm properties -----------------*- C++ -*--====//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the MapipMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef Mapip_MCASM_INFO_H
#define Mapip_MCASM_INFO_H

#include "llvm/MC/MCAsmInfo.h"

namespace llvm {
  class Target;
  class StringRef;

  struct MapipMCAsmInfo : public MCAsmInfo {
    explicit MapipMCAsmInfo(const Target &T, const StringRef &TT);
  };
} // namespace llvm

#endif // Mapip_MCASM_INFO_H
