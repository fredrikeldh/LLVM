#include "MapipMachine.h"

using namespace llvm;

MapipMachine::MapipMachine(const Target& T, StringRef TT,
	StringRef CPU, StringRef FS,
	Reloc::Model RM, CodeModel::Model CM)
: LLVMTargetMachine(T, TT, CPU, FS, RM, CM)
{
}

void MapipMachine::dummy() {
}
