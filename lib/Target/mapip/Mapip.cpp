#include "Mapip.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCCodeGenInfo.h"
#include "MapipMachine.h"
#include "MapipRegisterInfo.h"
#include "MapipInstrInfo.h"
#include "MapipAsmInfo.h"

using namespace llvm;

extern "C" void LLVMInitializeMapipTarget() {
	// Register the target.
	RegisterTargetMachine<MapipMachine> X(TheMapipTarget);
}

static MCAsmInfo* createMapipMCAsmInfo(const llvm::Target&, llvm::StringRef) {
	MCAsmInfo* X = new MCAsmInfo();
	InitMapipMCAsmInfo(X);
	return X;
}
static MCInstrInfo *createMapipMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitMapipMCInstrInfo(X);
  return X;
}
static MCRegisterInfo* createMapipMCRegisterInfo(llvm::StringRef TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitMapipMCRegisterInfo(X);
  return X;
}
/*static MCSubtargetInfo *createMapipMCSubtargetInfo(StringRef TT, StringRef CPU,
	StringRef FS)
{
  MCSubtargetInfo *X = new MCSubtargetInfo();
  InitMapipMCSubtargetInfo(X, TT, CPU, FS);
  return X;
}*/
static MCCodeGenInfo *createMapipMCCodeGenInfo(StringRef TT, Reloc::Model RM,
	CodeModel::Model CM)
{
  MCCodeGenInfo *X = new MCCodeGenInfo();
  X->InitMCCodeGenInfo(RM, CM);
  return X;
}

extern "C" void LLVMInitializeMapipTargetMC() {
  // Register MC info.
	// Failure to do this can cause crashes due to NULL pointer dereferences.
  TargetRegistry::RegisterMCAsmInfo(TheMapipTarget, createMapipMCAsmInfo);
  TargetRegistry::RegisterMCCodeGenInfo(TheMapipTarget, createMapipMCCodeGenInfo);
  TargetRegistry::RegisterMCInstrInfo(TheMapipTarget, createMapipMCInstrInfo);
  TargetRegistry::RegisterMCRegInfo(TheMapipTarget, &createMapipMCRegisterInfo);
  //TargetRegistry::RegisterMCSubtargetInfo(TheMapipTarget, createMapipMCSubtargetInfo);
}
