add_llvm_library(LLVMliveness MODULE BUILDTREE_ONLY
  Liveness.cpp hw5-util.h tee.h
  DEPENDS
  intrinsics_gen
  PLUGIN_TOOL
  opt
)
