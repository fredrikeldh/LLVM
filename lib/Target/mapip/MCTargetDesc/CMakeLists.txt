add_llvm_library(LLVMMapipDesc
  MapipMCTargetDesc.cpp
  MapipMCAsmInfo.cpp
  )

add_llvm_library_dependencies(LLVMMapipDesc
  LLVMMapipInfo
  LLVMMC
  )

add_dependencies(LLVMMapipDesc MapipCommonTableGen)
