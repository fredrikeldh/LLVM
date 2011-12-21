//===- MapipRelocations.h - Mapip Code Relocations --------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the Mapip target-specific relocation types.
//
//===----------------------------------------------------------------------===//

#ifndef MAPIPRELOCATIONS_H
#define MAPIPRELOCATIONS_H

#include "llvm/CodeGen/MachineRelocation.h"

namespace llvm {
  namespace Mapip {
    enum RelocationType {
      reloc_literal,
      reloc_gprellow,
      reloc_gprelhigh,
      reloc_gpdist,
      reloc_bsr
    };
  }
}

#endif
