##===- lib/Target/Mapip/Makefile ---------------------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

LEVEL = ../../..
LIBRARYNAME = LLVMMapipCodeGen
TARGET = Mapip

# Make sure that tblgen is run, first thing.
BUILT_SOURCES = MapipGenAsmWriter.inc \
	MapipGenDAGISel.inc \
	MapipGenInstrInfo.inc \
	MapipGenRegisterInfo.inc \
	MapipGenSubtargetInfo.inc \

DIRS = TargetInfo

CPPFLAGS = -Werror

include $(LEVEL)/Makefile.common
