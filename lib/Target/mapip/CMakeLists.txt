
add_llvm_target(MapipCodeGen
)

add_subdirectory(TargetInfo)

set_property(GLOBAL
	PROPERTY COMPILE_FLAGS "-Werror"
)
