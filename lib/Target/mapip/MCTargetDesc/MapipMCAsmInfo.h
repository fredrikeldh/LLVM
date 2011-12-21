//=====-- MapipMCAsmInfo.h - Mapip asm properties -------------*- C++ -*--====//
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

#ifndef MAPIPTARGETASMINFO_H
#define MAPIPTARGETASMINFO_H

#include "llvm/ADT/StringRef.h"
#include "llvm/MC/MCAsmInfo.h"

namespace llvm {
  class Target;

  struct MapipMCAsmInfo : public MCAsmInfo {
    explicit MapipMCAsmInfo(const Target &T, StringRef TT);
  };

} // namespace llvm

#endif
