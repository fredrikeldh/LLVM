//===-- MapipMCTargetDesc.h - Mapip Target Descriptions ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides Mapip specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef MAPIPMCTARGETDESC_H
#define MAPIPMCTARGETDESC_H

namespace llvm {
class MCSubtargetInfo;
class Target;
class StringRef;

extern Target TheMapipTarget;

} // End llvm namespace

// Defines symbolic names for Mapip registers.  This defines a mapping from
// register name to register number.
//
#define GET_REGINFO_ENUM
#include "MapipGenRegisterInfo.inc"

// Defines symbolic names for the Mapip instructions.
//
#define GET_INSTRINFO_ENUM
#include "MapipGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "MapipGenSubtargetInfo.inc"

#endif
