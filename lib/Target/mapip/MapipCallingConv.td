//===- MapipCallingConv.td - Calling Conventions for Mapip -*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// This describes the calling conventions for Mapip architecture.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Mapip Return Value Calling Convention
//===----------------------------------------------------------------------===//
def RetCC_Mapip : CallingConv<[
  // i64 is returned in register R0
  // R1 is an llvm extension, I don't know what gcc does
  CCIfType<[i64], CCAssignToReg<[R0,R1]>>,

  // f32 / f64 are returned in F0/F1
  CCIfType<[f32, f64], CCAssignToReg<[F0, F1]>>
]>;

//===----------------------------------------------------------------------===//
// Mapip Argument Calling Conventions
//===----------------------------------------------------------------------===//
def CC_Mapip : CallingConv<[
  CCIfType<[i8, i16], CCPromoteToType<i32>>,

	// The first 6 arguments are passed in registers, whether integer or
  // floating-point
  CCIfType<[i32], CCAssignToReg<[R16, R17, R18, R19, R20, R21]>>,

  CCIfType<[f64], CCAssignToReg<[F16, F17, F18, F19, F20, F21]>>,

  // Stack slots are 4 bytes in size and 4-byte aligned.
  CCIfType<[i32, f64], CCAssignToStack<4, 4>>
]>;
