#include "MapipRegisterInfo.h"
#include "MapipRegisterInfoShared.h"

using namespace llvm;

struct SparcGenMCRegisterInfo : public MCRegisterInfo {
  explicit SparcGenMCRegisterInfo(const MCRegisterDesc *D);
};

static MCRegisterDesc SparcRegDesc[] = { // Descriptors
	{ "NOREG",	0,	0,	0 },
	{ "SP",	0,	0,	0 },
	{ "RT", 0, 0, 0 },
	{ "FR", 0, 0, 0 },
	{ "D0", 0, 0, 0 },
	{ "D1", 0, 0, 0 },
	{ "D2", 0, 0, 0 },
	{ "D3", 0, 0, 0 },
	{ "D4", 0, 0, 0 },
	{ "D5", 0, 0, 0 },
	{ "D6", 0, 0, 0 },
	{ "D7", 0, 0, 0 },
	{ "I0", 0, 0, 0 },
	{ "I1", 0, 0, 0 },
	{ "I2", 0, 0, 0 },
	{ "I3", 0, 0, 0 },
	{ "R0", 0, 0, 0 },
	{ "R1", 0, 0, 0 },
	{ "R2", 0, 0, 0 },
	{ "R3", 0, 0, 0 },
	{ "R4", 0, 0, 0 },
	{ "R5", 0, 0, 0 },
	{ "R6", 0, 0, 0 },
	{ "R7", 0, 0, 0 },
	{ "R8", 0, 0, 0 },
	{ "R9", 0, 0, 0 },
	{ "R10", 0, 0, 0 },
	{ "R11", 0, 0, 0 },
	{ "R12", 0, 0, 0 },
	{ "R13", 0, 0, 0 },
	{ "R14", 0, 0, 0 },
	{ "R15", 0, 0, 0 },
};

namespace {     // Register classes...
  // IntRegs Register Class...
  static const unsigned IntRegs[] = {
		SP::SP,
		SP::RT,
		SP::FR,
		SP::D0,
		SP::D1,
		SP::D2,
		SP::D3,
		SP::D4,
		SP::D5,
		SP::D6,
		SP::D7,
		SP::I0,
		SP::I1,
		SP::I2,
		SP::I3,
		SP::R0,
		SP::R1,
		SP::R2,
		SP::R3,
		SP::R4,
		SP::R5,
		SP::R6,
		SP::R7,
		SP::R8,
		SP::R9,
		SP::R10,
		SP::R11,
		SP::R12,
		SP::R13,
		SP::R14,
		SP::R15,
  };

  // IntRegs Bit set.
  static const unsigned char IntRegsBits[] = {
		0x7f, 0xff, 0xff, 0xff,
  };
}

static MCRegisterClass MapipMCRegisterClasses[] = {
  MCRegisterClass(SP::IntRegsRegClassID, "IntRegs", 4, 4, 1, 1, IntRegs, IntRegs + 32, IntRegsBits, sizeof(IntRegsBits)),
  //MCRegisterClass(SP::FPRegsRegClassID, "FPRegs", 4, 4, 1, 1, FPRegs, FPRegs + 32, FPRegsBits, sizeof(FPRegsBits)),
  //MCRegisterClass(SP::DFPRegsRegClassID, "DFPRegs", 8, 8, 1, 1, DFPRegs, DFPRegs + 16, DFPRegsBits, sizeof(DFPRegsBits)),
};


void InitMapipMCRegisterInfo(MCRegisterInfo* RI) {
  RI->InitMCRegisterInfo(SparcRegDesc, 32, SP::RT, MapipMCRegisterClasses, 1);

	RI->mapDwarfRegToLLVMReg(SP::SP, SP::SP, false );
	RI->mapDwarfRegToLLVMReg(SP::RT, SP::RT, false );
	RI->mapDwarfRegToLLVMReg(SP::FR, SP::FR, false );
	RI->mapDwarfRegToLLVMReg(SP::D0, SP::D0, false );
	RI->mapDwarfRegToLLVMReg(SP::D1, SP::D1, false );
	RI->mapDwarfRegToLLVMReg(SP::D2, SP::D2, false );
	RI->mapDwarfRegToLLVMReg(SP::D3, SP::D3, false );
	RI->mapDwarfRegToLLVMReg(SP::D4, SP::D4, false );
	RI->mapDwarfRegToLLVMReg(SP::D5, SP::D5, false );
	RI->mapDwarfRegToLLVMReg(SP::D6, SP::D6, false );
	RI->mapDwarfRegToLLVMReg(SP::D7, SP::D7, false );
	RI->mapDwarfRegToLLVMReg(SP::I0, SP::I0, false );
	RI->mapDwarfRegToLLVMReg(SP::I1, SP::I1, false );
	RI->mapDwarfRegToLLVMReg(SP::I2, SP::I2, false );
	RI->mapDwarfRegToLLVMReg(SP::I3, SP::I3, false );
	RI->mapDwarfRegToLLVMReg(SP::R0, SP::R0, false );
	RI->mapDwarfRegToLLVMReg(SP::R1, SP::R1, false );
	RI->mapDwarfRegToLLVMReg(SP::R2, SP::R2, false );
	RI->mapDwarfRegToLLVMReg(SP::R3, SP::R3, false );
	RI->mapDwarfRegToLLVMReg(SP::R4, SP::R4, false );
	RI->mapDwarfRegToLLVMReg(SP::R5, SP::R5, false );
	RI->mapDwarfRegToLLVMReg(SP::R6, SP::R6, false );
	RI->mapDwarfRegToLLVMReg(SP::R7, SP::R7, false );
	RI->mapDwarfRegToLLVMReg(SP::R8, SP::R8, false );
	RI->mapDwarfRegToLLVMReg(SP::R9, SP::R9, false );
	RI->mapDwarfRegToLLVMReg(SP::R10, SP::R10, false );
	RI->mapDwarfRegToLLVMReg(SP::R11, SP::R11, false );
	RI->mapDwarfRegToLLVMReg(SP::R12, SP::R12, false );
	RI->mapDwarfRegToLLVMReg(SP::R13, SP::R13, false );
	RI->mapDwarfRegToLLVMReg(SP::R14, SP::R14, false );
	RI->mapDwarfRegToLLVMReg(SP::R15, SP::R15, false );
}
