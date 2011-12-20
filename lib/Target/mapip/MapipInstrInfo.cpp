#include "MapipInstrInfo.h"
#include "MapipRegisterInfoShared.h"

using namespace SP;
using namespace MCOI;
using namespace MCID;

static const unsigned ImplicitSP[] = { SP, 0 };

// insufficient
static const MCOperandInfo OI_PUSH[] = {
	{ IntRegsRegClassID, 0, 0, OPERAND_REGISTER },
	{ -1, 0, 0, OPERAND_IMMEDIATE },
};

#define INSTRUCTIONS(m)\
	m(PUSH, (1 << ExtraSrcRegAllocReq) | (1 << MayStore), ImplicitSP, ImplicitSP, OI_PUSH)\
	m(POP)\
	m(CALL)\
	m(CALLI)\
	m(LDB)\
	m(STB)\
	m(LDH)\
	m(STH)\
	m(LDW)\
	m(STW)\
	m(LDI)\
	m(LDR)\
	m(ADD)\
	m(ADDI)\
	m(MUL)\
	m(MULI)\
	m(SUB)\
	m(SUBI)\
	m(AND)\
	m(ANDI)\
	m(OR)\
	m(ORI)\
	m(XOR)\
	m(XORI)\
	m(DIVU)\
	m(DIVUI)\
	m(DIV)\
	m(DIVI)\
	m(SLL)\
	m(SLLI)\
	m(SRA)\
	m(SRAI)\
	m(SRL)\
	m(SRLI)\
	m(NOT)\
	m(NEG)\
	m(RET)\
	m(JC_EQ)\
	m(JC_NE)\
	m(JC_GE)\
	m(JC_GEU)\
	m(JC_GT)\
	m(JC_GTU)\
	m(JC_LE)\
	m(JC_LEU)\
	m(JC_LT)\
	m(JC_LTU)\
	m(JPI)\
	m(JPR)\
	m(XB)\
	m(XH)\
	m(SYSCALL)\
	m(CASE)\
	m(FAR)\

#define ENUM_INSTRUCTION_ELEM(name, flags, implicitUse, implicitDef, operandInfo) _##name,
enum
{
	_NUL = 0,
	INSTRUCTIONS(ENUM_INSTRUCTION_ELEM)
	_ENDOP
};

#define ID_INSTRUCTION(name, flags, implicitUse, implicitDef, operandInfo) {\
	_##name, sizeof(operandInfo) / sizeof(MCOperandInfo), 0, 0, 0, name, flags, 0,\
	implicitUse, implicitDef, operandInfo},


static MCInstrDesc sInstrDesc[] = {
  // Opcode, NumOperands, NumDefs, SchedClass, Size, Name, Flags, TSFlags, ImplicitUses, ImplicitDefs, OpInfo
	//{_PUSH, sizeof(OI_PUSH) / sizeof(MCOperandInfo), 0, 0, 0, "PUSH", (1 << ExtraSrcRegAllocReq), 0, NULL, ImplicitSP, OI_PUSH},
	INSTRUCTIONS(ID_INSTRUCTION)
};

void InitMapipMCInstrInfo(llvm::MCInstrInfo* II) {
	II->InitMCInstrInfo(sInstrDesc, sizeof(sInstrDesc) / sizeof(MCInstrDesc));
}
