//===- MapipInstrInfo.td - Mapip Instruction defs -------------*- tblgen-*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file describes the Mapip instructions in TableGen format.
//
//===----------------------------------------------------------------------===//

include "MapipInstrFormats.td"

//===----------------------------------------------------------------------===//
// Instructions
//===----------------------------------------------------------------------===//

def LDI : InstMapip<11, (outs GPR:$dst), (ins Imm:$imm),
	"ld $dst, $imm", [(set GPR:$dst, imm32:$imm)]>;

def LDR : InstMapip<12, (outs GPR:$dst), (ins GPR:$src),
	"ld $dst, $src", [(set GPR:$dst, GPR:$src)]>;

def LDW : InstMapip<9, (outs GPR:$dst), (ins Memri:$addr),
	"ld $dst, $addr", [(set (i32 GPR:$dst), (load ADDRri:$addr))]>;

def STW : InstMapip<10, (outs), (ins Memri:$addr, GPR:$src),
	"ld $addr, $src", [(store GPR:$src, ADDRri:$addr)]>;

defm ADD : InstAluMulti<13, 14, "add", add>;
defm MUL : InstAluMulti<15, 16, "mul", mul>;
defm SUB : InstAluMulti<17, 18, "sub", sub>;
defm AND : InstAluMulti<19, 20, "and", and>;
defm OR : InstAluMulti<21, 22, "or", or>;
defm XOR : InstAluMulti<23, 24, "xor", xor>;
defm DIVU : InstAluMulti<25, 26, "divu", udiv>;
defm DIV : InstAluMulti<27, 28, "div", sdiv>;
defm SLL : InstAluMulti<29, 30, "sll", shl>;
defm SRA : InstAluMulti<31, 32, "sra", sra>;
defm SRL : InstAluMulti<33, 34, "srl", srl>;

def NOT : InstMapip<35, (outs GPR:$dst), (ins GPR:$src),
	"not $dst, $src", [(set GPR:$dst, (not GPR:$src))]>;

def NEG : InstMapip<36, (outs GPR:$dst), (ins GPR:$src),
	"neg $dst, $src", [(set GPR:$dst, (ineg GPR:$src))]>;

def RET : InstMapip<37, (outs), (ins), "ret", []> {
	let isReturn=1;
	let isTerminator=1;
	let isBarrier=1;
	let Uses=[R2];
}

// howto specify that call address is in src?
def CALL : InstMapip<3, (outs), (ins GPR:$src), "call $src", []> {
	let isCall=1;
	let Defs=[R2];
}

def CALLI : InstMapip<4, (outs), (ins brtarget16:$imm), "call $imm", []> {
	let isCall=1;
	let Defs=[R2];
}

def FAR_CALLI : InstMapip<4, (outs), (ins brtarget24:$imm), "call $imm", []> {
	let isCall=1;
	let Defs=[R2];
}

def JPI : InstMapip<48, (outs), (ins brtarget16:$imm), "jmp $imm", []> {
	let isBranch = 1;
	let isBarrier = 1;
	let isTerminator = 1;
}

def FAR_JPI : InstMapip<48, (outs), (ins brtarget24:$imm), "jmp $imm", []> {
	let isBranch = 1;
	let isBarrier = 1;
	let isTerminator = 1;
}

def JPR : InstMapip<49, (outs), (ins GPR:$dst), "jpr $dst", []> {
	let isIndirectBranch = 1;
	let isBarrier = 1;
	let isTerminator = 1;
}

defm JC_EQ : InstJC<38, "jc_eq", seteq>;
defm JC_NE : InstJC<39, "jc_ne", setne>;
defm JC_GE : InstJC<40, "jc_ge", setge>;
defm JC_GEU : InstJC<41, "jc_geu", setuge>;
defm JC_GT : InstJC<42, "jc_gt", setgt>;
defm JC_GTU : InstJC<43, "jc_gtu", setugt>;
defm JC_LE : InstJC<44, "jc_le", setle>;
defm JC_LEU : InstJC<45, "jc_leu", setule>;
defm JC_LT : InstJC<46, "jc_lt", setlt>;
defm JC_LTU : InstJC<47, "jc_ltu", setult>;

/*	// too tricky for now
// todo: specify 8-bit
def XB : InstMapip<50, (outs GPR:$dst), (ins GPR:$src),
	"xb $dst, $src", [(set GPR:$dst, (sext GPR:$src))]>;

// todo: specify 16-bit
def XH : InstMapip<51, (outs GPR:$dst), (ins GPR:$src),
	"xh $dst, $src", [(set GPR:$dst, (sext GPR:$src))]>;
*/

def SYSCALL : InstMapip<52, (outs), (ins Imm8:$num), "syscall $num", []>;

// really tricky. also not really needed.
//def CASE : InstMapip<53>;

// prefix instruction. changes brtarget16 to brtarget24.
//def FAR : InstMapip<54>;
