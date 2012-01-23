//===- MapipSubtarget.cpp - Mapip Subtarget Information -----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Mapip specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "MapipSubtarget.h"
#include "Mapip.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "MapipGenSubtargetInfo.inc"

using namespace llvm;

MapipSubtarget::MapipSubtarget(const std::string &TT, const std::string &CPU,
	const std::string &FS, bool little) :
MapipGenSubtargetInfo(TT, CPU, FS),
MapipArchVersion(Mapip32), MapipABI(MosyncABI), IsLittle(little),
IsGP64bit(false), HasVFPU(false),
IsLinux(true), HasSEInReg(false), HasCondMov(false), HasMulDivAdd(false),
HasMinMax(false), HasSwap(false), HasBitCount(false)
{
	std::string CPUName = CPU;
	if (CPUName.empty())
		CPUName = "Mapip32r1";

	// Parse features string.
	ParseSubtargetFeatures(CPUName, FS);

	// Initialize scheduling itinerary for the specified CPU.
	InstrItins = getInstrItineraryForCPU(CPUName);

	// Check if Architecture and ABI are compatible.
	assert(((!hasMapip64() && (isABI_O32() || isABI_EABI())) ||
		(hasMapip64() && (isABI_N32() || isABI_N64()))) &&
		"Invalid  Arch & ABI pair.");

	// Is the target system Linux ?
	if (TT.find("linux") == std::string::npos)
		IsLinux = false;
}
