include_directories( ${CMAKE_CURRENT_BINARY_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/.. )

add_llvm_library(LLVMMapipInfo
  MapipTargetInfo.cpp
  )

add_llvm_library_dependencies(LLVMMapipInfo
  LLVMMC
  LLVMSupport
  LLVMTarget
  )

add_dependencies(LLVMMapipInfo MapipCommonTableGen)
