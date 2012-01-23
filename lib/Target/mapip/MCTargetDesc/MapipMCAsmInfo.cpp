//===-- MapipMCAsmInfo.cpp - Mapip asm properties -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations of the MapipMCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "MapipMCAsmInfo.h"
#include "llvm/ADT/Triple.h"

using namespace llvm;

MapipELFMCAsmInfo::MapipELFMCAsmInfo(const Target &T, StringRef TT) {
  IsLittleEndian = false;
  Triple TheTriple(TT);

  Data16bitsDirective = "\t.half\t";
  Data32bitsDirective = "\t.word\t";
  Data64bitsDirective = 0;  // .xword is only supported by V9.
  ZeroDirective = "\t.skip\t";
  CommentString = "!";
  HasLEB128 = true;
  SupportsDebugInformation = true;

  SunStyleELFSectionSwitchSyntax = true;
  UsesELFSectionDirectiveForBSS = true;

  WeakRefDirective = "\t.weak\t";

  PrivateGlobalPrefix = ".L";
}
