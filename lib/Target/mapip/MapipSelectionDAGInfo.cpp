//===-- MapipSelectionDAGInfo.cpp - Mapip SelectionDAG Info -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the MapipSelectionDAGInfo class.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "mapip-selectiondag-info"
#include "MapipTargetMachine.h"
using namespace llvm;

MapipSelectionDAGInfo::MapipSelectionDAGInfo(const MapipTargetMachine &TM)
	: TargetSelectionDAGInfo(TM) {
}

MapipSelectionDAGInfo::~MapipSelectionDAGInfo() {
}
