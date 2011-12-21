set(LLVM_TARGET_DEFINITIONS Mapip.td)

llvm_tablegen(MapipGenRegisterInfo.inc -gen-register-info)
llvm_tablegen(MapipGenInstrInfo.inc -gen-instr-info)
llvm_tablegen(MapipGenAsmWriter.inc -gen-asm-writer)
llvm_tablegen(MapipGenDAGISel.inc -gen-dag-isel)
llvm_tablegen(MapipGenCallingConv.inc -gen-callingconv)
llvm_tablegen(MapipGenSubtargetInfo.inc -gen-subtarget)
add_public_tablegen_target(MapipCommonTableGen)

add_llvm_target(MapipCodeGen
  MapipAsmPrinter.cpp
  MapipBranchSelector.cpp
  MapipInstrInfo.cpp
  MapipISelDAGToDAG.cpp
  MapipISelLowering.cpp
  MapipFrameLowering.cpp
  MapipLLRP.cpp
  MapipRegisterInfo.cpp
  MapipSubtarget.cpp
  MapipTargetMachine.cpp
  MapipSelectionDAGInfo.cpp
  )

add_llvm_library_dependencies(LLVMMapipCodeGen
  LLVMMapipDesc
  LLVMMapipInfo
  LLVMAsmPrinter
  LLVMCodeGen
  LLVMCore
  LLVMMC
  LLVMSelectionDAG
  LLVMSupport
  LLVMTarget
  )

add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)

set_property(GLOBAL
	PROPERTY COMPILE_FLAGS "-Werror"
)
