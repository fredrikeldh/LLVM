//===- MapipRegisterInfo.td - Mapip Register defs ----------------*- tblgen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the Mapip register file
//===----------------------------------------------------------------------===//

class MapipReg<string n> : Register<n> {
  let Namespace = "Mapip";
}

///===- 32-bit Integer Registers ------------------------------------------===//

def ZERO : MapipReg<"zero">;
def SP : MapipReg<"sp">;
def RT : MapipReg<"rt">;
def FP : MapipReg<"fp">;
def D0 : MapipReg<"d0">;
def D1 : MapipReg<"d1">;
def D2 : MapipReg<"d2">;
def D3 : MapipReg<"d3">;
def D4 : MapipReg<"d4">;
def D5 : MapipReg<"d5">;
def D6 : MapipReg<"d6">;
def D7 : MapipReg<"d7">;
def P0 : MapipReg<"p0">;
def P1 : MapipReg<"p1">;
def P2 : MapipReg<"p2">;
def P3 : MapipReg<"p3">;
def R0 : MapipReg<"r0">;
def R1 : MapipReg<"r1">;
def R2 : MapipReg<"r2">;
def R3 : MapipReg<"r3">;
def R4 : MapipReg<"r4">;
def R5 : MapipReg<"r5">;
def R6 : MapipReg<"r6">;
def R7 : MapipReg<"r7">;
def R8 : MapipReg<"r8">;
def R9 : MapipReg<"r9">;
def R10 : MapipReg<"r10">;
def R11 : MapipReg<"r11">;
def R12 : MapipReg<"r12">;
def R13 : MapipReg<"r13">;
def R14 : MapipReg<"r14">;
def R15 : MapipReg<"r15">;

/* todo
///===- 64-bit Floating-Point Registers -----------------------------------===//

def FD0  : MapipReg<"fd0">;
def FD1  : MapipReg<"fd1">;
def FD2  : MapipReg<"fd2">;
def FD3  : MapipReg<"fd3">;
def FD4  : MapipReg<"fd4">;
def FD5  : MapipReg<"fd5">;
def FD6  : MapipReg<"fd6">;
def FD7  : MapipReg<"fd7">;
def FD8  : MapipReg<"fd8">;
def FD9  : MapipReg<"fd9">;
def FD10 : MapipReg<"fd10">;
def FD11 : MapipReg<"fd11">;
def FD12 : MapipReg<"fd12">;
def FD13 : MapipReg<"fd13">;
def FD14 : MapipReg<"fd14">;
def FD15 : MapipReg<"fd15">;
def FD16 : MapipReg<"fd16">;
def FD17 : MapipReg<"fd17">;
def FD18 : MapipReg<"fd18">;
def FD19 : MapipReg<"fd19">;
def FD20 : MapipReg<"fd20">;
def FD21 : MapipReg<"fd21">;
def FD22 : MapipReg<"fd22">;
def FD23 : MapipReg<"fd23">;
def FD24 : MapipReg<"fd24">;
def FD25 : MapipReg<"fd25">;
def FD26 : MapipReg<"fd26">;
def FD27 : MapipReg<"fd27">;
def FD28 : MapipReg<"fd28">;
def FD29 : MapipReg<"fd29">;
def FD30 : MapipReg<"fd30">;
def FD31 : MapipReg<"fd31">;
*/

//===----------------------------------------------------------------------===//
//  Register classes
//===----------------------------------------------------------------------===//

// 32-bit signed integer
def GPR : RegisterClass<"Mapip", [i32], 32,
	(add ZERO, SP, RT, FP,
	D0, D1, D2, D3, D4, D5, D6, D7,
	P0, P1, P2, P3,
	R0, R1, R2, R3, R4, R5, R6, R7,
	R8, R9, R10, R11, R12, R13, R14, R15)>;

/* todo
// 64-bit floating point
def Regf64 : RegisterClass<"Mapip", [f64], 64,
	(add FD0, FD1, FD2, FD3, FD4, FD5, FD6, FD7,
	FD8, FD9, FD10, FD11, FD12, FD13, FD14, FD15,
	FD16, FD17, FD18, FD19, FD20, FD21, FD22, FD23,
	FD24, FD25, FD26, FD27, FD28, FD29, FD30, FD31)>;
*/
