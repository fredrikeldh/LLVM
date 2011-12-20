set(LLVM_TARGET_DEFINITIONS Mapip.td)

tablegen(MapipGenAsmWriter.inc -gen-asm-writer)
tablegen(MapipGenDAGISel.inc -gen-dag-isel)
tablegen(MapipGenInstrInfo.inc -gen-instr-desc)
tablegen(MapipGenInstrNames.inc -gen-instr-enums)
tablegen(MapipGenRegisterInfo.inc -gen-register-desc)
tablegen(MapipGenRegisterInfo.h.inc -gen-register-desc-header)
tablegen(MapipGenRegisterNames.inc -gen-register-enums)
tablegen(MapipGenSubtarget.inc -gen-subtarget)

add_llvm_target(MapipCodeGen
	MapipAsmPrinter.cpp
	MapipISelDAGToDAG.cpp
	MapipISelLowering.cpp
	MapipInstrInfo.cpp
	MapipFrameLowering.cpp
	MapipMCAsmInfo.cpp
	MapipMCAsmStreamer.cpp
	MapipMFInfoExtract.cpp
	MapipRegisterInfo.cpp
	MapipSubtarget.cpp
	MapipTargetMachine.cpp
)

add_subdirectory(TargetInfo)

set_property(GLOBAL
	PROPERTY COMPILE_FLAGS "-Werror"
)
