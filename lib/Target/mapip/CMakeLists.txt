set(LLVM_TARGET_DEFINITIONS Mapip.td)

tablegen(MapipGenAsmWriter.inc -gen-asm-writer)
tablegen(MapipGenDAGISel.inc -gen-dag-isel)
tablegen(MapipGenInstrInfo.inc -gen-instr-desc)
tablegen(MapipGenRegisterInfo.inc -gen-register-desc)
tablegen(MapipGenSubtargetInfo.inc -gen-subtarget)
tablegen(MapipGenCallingConv.inc -gen-callingconv)

add_llvm_target(MapipCodeGen
	MapipAsmPrinter.cpp
	MapipInstrInfo.cpp
	MapipISelDAGToDAG.cpp
	MapipISelLowering.cpp
	MapipFrameLowering.cpp
	MapipRegisterInfo.cpp
	MapipSubtarget.cpp
	MapipTargetMachine.cpp
)

add_subdirectory(TargetInfo)

set_property(GLOBAL
	PROPERTY COMPILE_FLAGS "-Werror"
)
