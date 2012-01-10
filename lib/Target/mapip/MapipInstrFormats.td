//===- MapipInstrFormats.td - Mapip Instruction Formats ------*- tblgen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

include "MapipRegisterInfo.td"

//===----------------------------------------------------------------------===//
// Operand types
//===----------------------------------------------------------------------===//

def Imm : Operand<i32>;
def Imm16 : Operand<i16>;
def Imm8 : Operand<i8>;

// Immediate value
def immSExt16 : PatLeaf<(imm), [{ return isInt<16>(N->getSExtValue()); }]>;
def imm32 : PatLeaf<(imm), [{ return isInt<32>(N->getSExtValue()); }]>;

// Memory address, register + immediate.
def Memri : Operand<i32> {
  let PrintMethod = "printMemOperand";
  let MIOperandInfo = (ops GPR, Imm);
}
def ADDRri : ComplexPattern<i32, 2, "SelectADDRri", [frameindex], []>;

//def GPR : Regi32;

def brtarget24 : Operand<OtherVT>;
def brtarget16 : Operand<OtherVT>;

//===----------------------------------------------------------------------===//
// Instruction types
//===----------------------------------------------------------------------===//

class InstMapip<bits<7> op, dag oops, dag iops, string asmstr, list<dag> pattern>
: Instruction {
	let Namespace = "Mapip";
	bits<7> opcode = op;
	dag OutOperandList = oops;
	dag InOperandList = iops;
	let AsmString = asmstr;
	let Pattern = pattern;
}

class InstAluReg<bits<7> op, string instr_asm, SDNode opNode>
: InstMapip<op, (outs GPR:$dst), (ins GPR:$a, GPR:$src),
	!strconcat(instr_asm, " $dst, $src"),
	[(set GPR:$dst, (opNode GPR:$a, GPR:$src))]>
{
	let Constraints = "$a = $dst";
}

class InstAluImm<bits<7> op, string instr_asm, SDNode opNode>
: InstMapip<op, (outs GPR:$dst), (ins GPR:$a, Imm:$imm),
	!strconcat(instr_asm, " $dst, $imm"),
	[(set GPR:$dst, (opNode GPR:$a, imm32:$imm))]>
{
	let Constraints = "$a = $dst";
}

multiclass InstAluMulti<bits<7> regOp, bits<7> immOp, string instr_asm, SDNode opNode> {
  def rr : InstAluReg<regOp, instr_asm, opNode>;
  def ri : InstAluImm<immOp, !strconcat(instr_asm, "i"), opNode>;
}

// Jump Conditional

class InstJCBase<bits<7> op, string instr_asm, PatFrag cond_op, Operand targetType>
: InstMapip<op, (outs), (ins GPR:$a, GPR:$b, targetType:$target),
	!strconcat(instr_asm, " $a, $b, $target"),
	[(brcond (cond_op GPR:$a, GPR:$b), bb:$target)]>
{
	let isBranch = 1;
}

class InstJC16<bits<7> op, string instr_asm, PatFrag cond_op>
: InstJCBase<op, instr_asm, cond_op, brtarget16>;

class InstJC24<bits<7> op, string instr_asm, PatFrag cond_op>
: InstJCBase<op, instr_asm, cond_op, brtarget24>;

multiclass InstJC<bits<7> op, string instr_asm, PatFrag cond_op> {
  def jc16 : InstJC16<op, instr_asm, cond_op>;
  def jc24 : InstJC24<op, !strconcat("far ", instr_asm), cond_op>;
}
