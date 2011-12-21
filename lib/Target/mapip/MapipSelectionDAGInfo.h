//===-- MapipSelectionDAGInfo.h - Mapip SelectionDAG Info -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the Mapip subclass for TargetSelectionDAGInfo.
//
//===----------------------------------------------------------------------===//

#ifndef MAPIPSELECTIONDAGINFO_H
#define MAPIPSELECTIONDAGINFO_H

#include "llvm/Target/TargetSelectionDAGInfo.h"

namespace llvm {

class MapipTargetMachine;

class MapipSelectionDAGInfo : public TargetSelectionDAGInfo {
public:
  explicit MapipSelectionDAGInfo(const MapipTargetMachine &TM);
  ~MapipSelectionDAGInfo();
};

}

#endif
