#include <string>
#include <iostream>
#include <fstream>
#include <functional>

#include "CPU.h"
#include "definitions.h"

using namespace std;

s8 toSigned(u8 b) { return b < 128 ? b : b - 256; }
u8 hi(u8 val) { return val >> 4; }
u8 lo(u8 val) { return val & 0xF; }

bool checkHalfCarry(u8 a, u8 b) {
	return (((a & 0xf) + (b & 0xf)) & 0x10) == 0x10;
}
bool checkCarry(u8 a, u8 b) {
	int res = a + b;
	return ((a ^ b ^ res) & 0x100) != 0;
}

CPU::CPU() {
	u8* A = &(AF.high);
	u8* F = &(AF.low);
	u8* B = &(BC.high);
	u8* C = &(BC.low);
	u8* D = &(DE.high);
	u8* E = &(DE.low);
	u8* H = &(HL.high);
	u8* L = &(HL.low);
	auto ZERO = COND(FLAGS::Z);
	auto NZ = COND(FLAGS::Z, 0);
	auto CARRY = COND(FLAGS::C);
	auto NC = COND(FLAGS::C, 0);
	auto TRUE = COND(FLAGS::U1, 0);

	lookup = {
	{
		{{"NOP", NOP(), 1},				{"LD BC, d16", LD(&BC), 3},		{"LD (BC), A", LD(&BC, A), 2},		{"INC BC", INC(&BC), 2},		{"INC B", INC(B), 1},			{"DEC B", DEC(B), 1},			{"LD B, d8", LD(B), 2},				{"RLCA", RLCA(), 1},			{"LD (a16), SP", STSP(&SP), 5},		{"ADD HL, BC", ADD(&HL, &BC), 2},	{"LD A, (BC)", LD(A, &BC), 2},		{"DEC BC", DEC(&BC), 2},		{"INC C", INC(C), 1},				{"DEC C", DEC(C), 1},			{"LD C, d8", LD(C), 2},				{"RRCA", RRCA(), 1}},
		{{"STOP", STOP(), 1},			{"LD DE, d16",  LD(&DE), 3},	{"LD (DE), A", LD(&DE, A), 2},		{"INC DE", INC(&DE), 2},		{"INC D", INC(D), 1},			{"DEC D", DEC(D), 1},			{"LD D, d8", LD(D), 2},				{"RLA", RLA(), 1},				{"JR s8", JR(TRUE), 2},				{"ADD HL, DE", ADD(&HL, &DE), 2},	{"LD A, (DE)", LD(A, &DE), 2},		{"DEC DE", DEC(&DE), 2},		{"INC E", INC(E), 1},				{"DEC E", DEC(E), 1},			{"LD E, d8", LD(E), 2},				{"RRA", RRA(), 1}},
		{{"JR NZ, s8", JR(NZ), 2},		{"LD HL, d16", LD(&HL), 3},		{"LDI (HL+), A", LDI(&HL, A), 2},	{"INC HL", INC(&HL), 2},		{"INC H", INC(H), 1},			{"DEC H", DEC(H), 1},			{"LD H, d8", LD(H), 2},				{"DAA", DAA(), 1},				{"JR Z, s8", JR(ZERO), 2},			{"ADD HL, HL", ADD(&HL, &HL), 2},	{"LD A, (HL+)", LDI(A, &HL), 2},	{"DEC HL", DEC(&HL), 2},		{"INC L", INC(L), 1},				{"DEC L", DEC(L), 1},			{"LD L, d8", LD(L), 2},				{"CPL", CPL(A), 1}},
		{{"JR NC, s8", JR(NC), 2},		{"LD SP, d16", LD(&SP), 3},		{"LDD (HL-), A", LDD(&HL, A), 2},	{"INC SP", INC(&SP), 2},		{"INC (HL)", INC(), 3},			{"DEC (HL)", DEC(), 3},			{"LD (HL), d8", ST(&HL), 3},		{"SCF", SCF(), 1},				{"JR C, s8", JR(CARRY), 2},			{"ADD HL, SP", ADD(&HL, &SP), 2},	{"LD A, (HL-)", LDD(A, &HL), 2},	{"DEC SP", DEC(&SP), 2},		{"INC A", INC(A), 1},				{"DEC A", DEC(A), 1},			{"LD A, d8", LD(A), 2},				{"CCF", CCF(), 1}},
		{{"LD B, B", LD(B, B), 1},		{"LD B, C", LD(B, C), 1},		{"LD B, D", LD(B, D), 1},			{"LD B, E", LD(B, E), 1},		{"LD B, H", LD(B, H), 1},		{"LD B, L", LD(B, L), 1},		{"LD B, (HL)", LD(B, &HL), 2},		{"LD B, A", LD(B, A), 1},		{"LD C, B", LD(C, B), 1},			{"LD C, C", LD(C, C), 1},			{"LD C, D", LD(C, D), 1},			{"LD C, E",LD(C, E), 1},		{"LD C, H", LD(C, H), 1},			{"LD C, L", LD(C, L), 1},		{"LD C, (HL)", LD(C, &HL), 2},		{"LD C, A", LD(C, A), 1}},
		{{"LD D, B", LD(D, B), 1},		{"LD D, C", LD(D, C), 1},		{"LD D, D", LD(D, D), 1},			{"LD D, E", LD(D, E), 1},		{"LD D, H", LD(D, H), 1},		{"LD D, L", LD(D, L), 1},		{"LD D, (HL)", LD(D, &HL), 2},		{"LD D, A", LD(D, A), 1},		{"LD E, B", LD(E, B), 1},			{"LD E, C", LD(E, C), 1},			{"LD E, D", LD(E, D), 1},			{"LD E, E", LD(E, E), 1},		{"LD E, H", LD(E, H), 1},			{"LD E, L", LD(E, L), 1},		{"LD E, (HL)", LD(E, &HL), 2},		{"LD E, A", LD(E, A), 1}},
		{{"LD H, B", LD(H, B), 1},		{"LD H, C", LD(H, C), 1},		{"LD H, D", LD(H, D), 1},			{"LD H, E", LD(H, E), 1},		{"LD H, H", LD(H, H), 1},		{"LD H, L", LD(H, L), 1},		{"LD H, (HL)", LD(H, &HL), 2},		{"LD H, A", LD(H, A), 1},		{"LD L, B", LD(L, B), 1},			{"LD L, C", LD(L, C), 1},			{"LD L, D", LD(L, D), 1},			{"LD L, E", LD(L, E), 1},		{"LD L, H", LD(L, H), 1},			{"LD L, L", LD(L, L), 1},		{"LD L, (HL)", LD(L, &HL), 2},		{"LD L, A", LD(L, A), 1}},
		{{"LD (HL), B", LD(&HL, B), 1},	{"LD (HL), C", LD(&HL, C), 1},	{"LD (HL), D", LD(&HL, D), 1},		{"LD (HL), E", LD(&HL, E), 1},	{"LD (HL), H", LD(&HL, H), 1},	{"LD (HL), L", LD(&HL, L), 1},	{"HALT", HALT(), 1},				{"LD (HL), A", LD(&HL, A), 1},	{"LD A, B", LD(A, B), 1},			{"LD A, C", LD(A, C), 1},			{"LD A, D", LD(A, D), 1},			{"LD A, E", LD(A, E), 1},		{"LD A, H", LD(A, H), 1},			{"LD A, L", LD(A, L), 1},		{"LD A, (HL)", LD(A, &HL), 2},		{"LD A, A", LD(A, A), 1}},
		{{"ADD A, B", ADD(B), 1},		{"ADD A, C", ADD(C), 1},		{"ADD A, D", ADD(D), 1},			{"ADD A, E", ADD(E), 1},		{"ADD A, H", ADD(H), 1},		{"ADD A, L", ADD(L), 1},		{"ADD A, (HL)", ADD(A, &HL), 2},	{"ADD A, A", ADD(A), 1},		{"ADC A, B", OP(B, ADC()), 2},		{"ADC A, C", OP(C, ADC()), 2},		{"ADC A, D", OP(D, ADC()), 2},		{"ADC A, E", OP(E, ADC()), 2},	{"ADC A, H", OP(H, ADC()), 2},		{"ADC A, L", OP(L, ADC()), 2},	{"ADC A, (HL)", OP(&HL, ADC()), 2},	{"ADC A, A", OP(A, ADC()), 2}},
		{{"SUB B", OP(B, SUB()), 1},	{"SUB C", OP(C, SUB()), 1},		{"SUB D", OP(D, SUB()), 1},			{"SUB E", OP(E, SUB()), 1},		{"SUB H", OP(H, SUB()), 1},		{"SUB L", OP(L, SUB()), 1},		{"SUB (HL)", OP(&HL, SUB()), 2},	{"SUB A", OP(A, SUB()), 1},		{"SBC A, B", OP(B, SBC()), 2},		{"SBC A, C", OP(C, SBC()), 2},		{"SBC A, D", OP(D, SBC()), 2},		{"SBC A, E", OP(E, SBC()), 2},	{"SBC A, H", OP(H, SBC()), 2},		{"SBC A, L", OP(L, SBC()), 2},	{"SBC A, (HL)", OP(&HL, SBC()), 2},	{"SBC A, A", OP(A, SBC()), 1}},
		{{"AND B", OP(B, AND()), 1},	{"AND C", OP(C, AND()), 1},		{"AND D", OP(D, AND()), 1},			{"AND E", OP(E, AND()), 1},		{"AND H", OP(H, AND()), 1},		{"AND L", OP(L, AND()), 1},		{"AND (HL)", OP(&HL, AND()), 2},	{"AND A", OP(A, AND()), 1},		{"XOR B", OP(B, XOR()), 1},			{"XOR C", OP(C, XOR()), 1},			{"XOR D", OP(D, XOR()), 1},			{"XOR E", OP(E, XOR()), 1},		{"XOR H", OP(H, XOR()), 1},			{"XOR L", OP(L, XOR()), 1},		{"XOR (HL)", OP(&HL, XOR()), 2},	{"XOR A", OP(A, XOR()), 1}},
		{{"OR B", OP(B, OR()), 1},		{"OR C", OP(C, OR()), 1},		{"OR D", OP(D, OR()), 1},			{"OR E", OP(E, OR()), 1},		{"OR H", OP(H, OR()), 1},		{"OR L", OP(L, OR()), 1},		{"OR (HL)", OP(&HL, OR()), 2},		{"OR A", OP(A, OR()), 1},		{"CP B", OP(B, CP()), 1},			{"CP C", OP(C, CP()), 1},			{"CP D", OP(D, CP()), 1},			{"CP E", OP(E, CP()), 1},		{"CP H", OP(H, CP()), 1},			{"CP L", OP(L, CP()), 1},		{"CP (HL)", OP(&HL, CP()), 2},		{"CP A", OP(A, CP()), 1}},
		{{"RET NZ", RET(NZ), 2},		{"POP BC", POP(&BC), 3},		{"JP NZ, a16", JP(NZ), 3},			{"JP a16", JP(TRUE), 4},		{"CALL NZ, a16", CALL(NZ), 3},	{"PUSH BC", PUSH(&BC), 4},		{"ADD A, d8", ADD(), 2},			{"RST 0", RST(0), 4},			{"RET Z", RET(ZERO), 2},			{"RET", RET(), 4},					{"JP Z, a16", JP(ZERO), 3},			{"???", XXX(), 0},				{"CALL Z, a16", CALL(ZERO), 3},		{"CALL a16", CALL(TRUE), 3},	{"ADC A, d8", OP(ADC()), 2},		{"RST 1", RST(1), 4}},
		{{"RET NC", RET(NC), 2},		{"POP DE", POP(&DE), 3},		{"JP NC, a16", JP(NC), 3},			{"???", XXX(), 0},				{"CALL NC, a16", CALL(NC), 3},	{"PUSH DE", PUSH(&DE), 4},		{"SUB d8", OP(SUB()), 2},			{"RST 2", RST(2), 4},			{"RET C", RET(CARRY), 2},			{"RETI", RETI(), 4},				{"JP C, a16", JP(CARRY), 3},		{"???", XXX(), 0},				{"CALL C, a16", CALL(CARRY), 3},	{"???", XXX(), 0},				{"SBC A, d8", OP(SBC()), 2},		{"RST 3", RST(3), 4}},
		{{"LDH (a8), A", STH(A), 3},	{"POP HL", POP(&HL), 3},		{"LD (C), A", STH(C, A), 2},		{"???", XXX(), 0},				{"???", XXX(), 0},				{"PUSH HL", PUSH(&HL), 4},		{"AND d8", OP(AND()), 2},			{"RST 4", RST(4), 4},			{"ADD SP, s8", ADD(&SP), 4},		{"JP HL", JP(&HL), 1},				{"LD (a16), A", STA(A), 4},			{"???", XXX(), 0},				{"???", XXX(), 0},					{"???", XXX(), 0},				{"XOR d8", OP(XOR()), 2},			{"RST 5", RST(5), 4}},
		{{"LDH A, (a8)", LDH(A), 3},	{"POP AF", POP(&AF), 3},		{"LD A, (C)", LDH(A, C), 2},		{"DI", DI(), 1},				{"???", XXX(), 0},				{"PUSH AF", PUSH(&AF), 4},		{"OR d8", OP(OR()), 2},				{"RST 6", RST(6), 4},			{"LD HL, SP+s8", LDHL(&HL, &SP),3},	{"LD SP, HL", LD(&SP, &HL), 2},		{"LD A, (a16)", LDA(A), 4},			{"EI", EI(), 1},				{"???", XXX(), 0},					{"???", XXX(), 0},				{"CP d8",OP(CP()), 2},				{"RST 7", RST(7), 4}}
	},
	{
		{{"RLC B", RLC(B), 2},			{"RLC C", RLC(C), 2},			{"RLC D", RLC(D), 2},				{"RLC E", RLC(E), 2},			{"RLC H", RLC(H), 2},			{"RLC L", RLC(L), 2},			{"RLC (HL)", RLC(&HL), 4},			{"RLC A", RLC(A), 2},			{"RRC B", RRC(B), 2},				{"RRC C", RRC(C), 2},				{"RRC D", RRC(D), 2},				{"RRC E", RRC(E), 2},			{"RRC H", RRC(H), 2},				{"RRC L", RRC(L), 2},			{"RRC (HL)", RRC(&HL), 4},			{"RRC A", RRC(A), 2}},
		{{"RL B", RL(B), 2},			{"RL C", RL(C), 2},				{"RL D", RL(D), 2},					{"RL E", RL(E), 2},				{"RL H", RL(H), 2},				{"RL L", RL(L), 2},				{"RL (HL)", RL(&HL), 4},			{"RL A", RL(A), 2},				{"RR B", RR(B), 2},					{"RR C", RR(C), 2},					{"RR D", RR(D), 2},					{"RR E", RR(E), 2},				{"RR H", RR(H), 2},					{"RR L", RR(L), 2},				{"RR (HL)", RR(&HL), 4},			{"RR A", RR(A), 2}},
		{{"SLA B", SLA(B), 2},			{"SLA C", SLA(C), 2},			{"SLA D", SLA(D), 2},				{"SLA E", SLA(E), 2},			{"SLA H", SLA(H), 2},			{"SLA L", SLA(L), 2},			{"SLA (HL)", SLA(&HL), 4},			{"SLA A", SLA(A), 2},			{"SRA B", SRA(B), 2},				{"SRA C", SRA(B), 2},				{"SRA D", SRA(D), 2},				{"SRA E", SRA(E), 2},			{"SRA H", SRA(H), 2},				{"SRA L", SRA(L), 2},			{"SRA (HL)", SRA(&HL), 4},			{"SRA A", SRA(A), 2}},
		{{"SWAP B", SWAP(B), 2},		{"SWAP C", SWAP(C), 2},			{"SWAP D", SWAP(D), 2},				{"SWAP E", SWAP(E), 2},			{"SWAP H", SWAP(H), 2},			{"SWAP L", SWAP(L), 2},			{"SWAP (HL)", SWAP(&HL), 4},		{"SWAP A", SWAP(A), 2},			{"SRL B", SRL(B), 2},				{"SRL C", SRL(C), 2},				{"SRL D", SRL(D), 2},				{"SRL E", SRL(E), 2},			{"SRL H", SRL(H), 2},				{"SRL L", SRL(L), 2},			{"SRL (HL)", SRL(&HL), 4},			{"SRL A", SRL(A), 2}},
		{{"BIT 0, B", BIT(0, B), 2},	{"BIT 0, C", BIT(0, C), 2},		{"BIT 0, D", BIT(0, D), 2},			{"BIT 0, E", BIT(0, E), 2},		{"BIT 0, H", BIT(0, H), 2},		{"BIT 0, L", BIT(0, L), 2},		{"BIT 0, (HL)", BIT(0, &HL), 4},	{"BIT 0, A", BIT(0, A), 2},		{"BIT 1, B", BIT(1, B), 2},			{"BIT 1, C", BIT(1, C), 2},			{"BIT 1, D", BIT(1, D), 2},			{"BIT 1, E", BIT(1, E), 2},		{"BIT 1, H", BIT(1, H), 2},			{"BIT 1, L", BIT(1, L), 2},		{"BIT 1, (HL)", BIT(1, &HL), 4},	{"BIT 1, A", BIT(1, A), 2}},
		{{"BIT 2, B", BIT(2, B), 2},	{"BIT 2, C", BIT(2, C), 2},		{"BIT 2, D", BIT(2, D), 2},			{"BIT 2, E", BIT(2, E), 2},		{"BIT 2, H", BIT(2, H), 2},		{"BIT 2, L", BIT(2, L), 2},		{"BIT 2, (HL)", BIT(2, &HL), 4},	{"BIT 2, A", BIT(2, A), 2},		{"BIT 3, B", BIT(3, B), 2},			{"BIT 3, C", BIT(3, C), 2},			{"BIT 3, D", BIT(3, D), 2},			{"BIT 3, E", BIT(3, E), 2},		{"BIT 3, H", BIT(3, H), 2},			{"BIT 3, L", BIT(3, L), 2},		{"BIT 3, (HL)", BIT(3, &HL), 4},	{"BIT 3, A", BIT(3, A), 2}},
		{{"BIT 4, B", BIT(4, B), 2},	{"BIT 4, C", BIT(4, C), 2},		{"BIT 4, D", BIT(4, D), 2},			{"BIT 4, E", BIT(4, E), 2},		{"BIT 4, H", BIT(4, H), 2},		{"BIT 4, L", BIT(4, L), 2},		{"BIT 4, (HL)", BIT(4, &HL), 4},	{"BIT 4, A", BIT(4, A), 2},		{"BIT 5, B", BIT(5, B), 2},			{"BIT 5, C", BIT(5, C), 2},			{"BIT 5, D", BIT(5, D), 2},			{"BIT 5, E", BIT(5, E), 2},		{"BIT 5, H", BIT(5, H), 2},			{"BIT 5, L", BIT(5, L), 2},		{"BIT 5, (HL)", BIT(5, &HL), 4},	{"BIT 5, A", BIT(5, A), 2}},
		{{"BIT 6, B", BIT(6, B), 2},	{"BIT 6, C", BIT(6, C), 2},		{"BIT 6, D", BIT(6, D), 2},			{"BIT 6, E", BIT(6, E), 2},		{"BIT 6, H", BIT(6, H), 2},		{"BIT 6, L", BIT(6, L), 2},		{"BIT 6, (HL)", BIT(6, &HL), 4},	{"BIT 6, A", BIT(6, A), 2},		{"BIT 7, B", BIT(7, B), 2},			{"BIT 7, C", BIT(7, C), 2},			{"BIT 7, D", BIT(7, D), 2},			{"BIT 7, E", BIT(7, E), 2},		{"BIT 7, H", BIT(7, H), 2},			{"BIT 7, L", BIT(7, L), 2},		{"BIT 7, (HL)", BIT(7, &HL), 4},	{"BIT 7, A", BIT(7, A), 2}},
		{{"RES 0, B", RES(0, B), 2},	{"RES 0, C", RES(0, C), 2},		{"RES 0, D", RES(0, D), 2},			{"RES 0, E", RES(0, E), 2},		{"RES 0, H", RES(0, H), 2},		{"RES 0, L", RES(0, L), 2},		{"RES 0, (HL)", RES(0, &HL), 4},	{"RES 0, A", RES(0, A), 2},		{"RES 1, B", RES(1, B), 2},			{"RES 1, C", RES(1, C), 2},			{"RES 1, D", RES(1, D), 2},			{"RES 1, E", RES(1, E), 2},		{"RES 1, H", RES(1, H), 2},			{"RES 1, L", RES(1, L), 2},		{"RES 1, (HL)", RES(1, &HL), 4},	{"RES 1, A", RES(1, A), 2}},
		{{"RES 2, B", RES(2, B), 2},	{"RES 2, C", RES(2, C), 2},		{"RES 2, D", RES(2, D), 2},			{"RES 2, E", RES(2, E), 2},		{"RES 2, H", RES(2, H), 2},		{"RES 2, L", RES(2, L), 2},		{"RES 2, (HL)", RES(2, &HL), 4},	{"RES 2, A", RES(2, A), 2},		{"RES 3, B", RES(3, B), 2},			{"RES 3, C", RES(3, C), 2},			{"RES 3, D", RES(3, D), 2},			{"RES 3, E", RES(3, E), 2},		{"RES 3, H", RES(3, H), 2},			{"RES 3, L", RES(3, L), 2},		{"RES 3, (HL)", RES(3, &HL), 4},	{"RES 3, A", RES(3, A), 2}},
		{{"RES 4, B", RES(4, B), 2},	{"RES 4, C", RES(4, C), 2},		{"RES 4, D", RES(4, D), 2},			{"RES 4, E", RES(4, E), 2},		{"RES 4, H", RES(4, H), 2},		{"RES 4, L", RES(4, L), 2},		{"RES 4, (HL)", RES(4, &HL), 4},	{"RES 4, A", RES(4, A), 2},		{"RES 5, B", RES(5, B), 2},			{"RES 5, C", RES(5, C), 2},			{"RES 5, D", RES(5, D), 2},			{"RES 5, E", RES(5, E), 2},		{"RES 5, H", RES(5, H), 2},			{"RES 5, L", RES(5, L), 2},		{"RES 5, (HL)", RES(5, &HL), 4},	{"RES 5, A", RES(5, A), 2}},
		{{"RES 6, B", RES(6, B), 2},	{"RES 6, C", RES(6, C), 2},		{"RES 6, D", RES(6, D), 2},			{"RES 6, E", RES(6, E), 2},		{"RES 6, H", RES(6, H), 2},		{"RES 6, L", RES(6, L), 2},		{"RES 6, (HL)", RES(6, &HL), 4},	{"RES 6, A", RES(6, A), 2},		{"RES 7, B", RES(7, B), 2},			{"RES 7, C", RES(7, C), 2},			{"RES 7, D", RES(7, D), 2},			{"RES 7, E", RES(7, E), 2},		{"RES 7, H", RES(7, H), 2},			{"RES 7, L", RES(7, L), 2},		{"RES 7, (HL)", RES(7, &HL), 4},	{"RES 7, A", RES(7, A), 2}},
		{{"SET 0, B", SET(0, B), 2},	{"SET 0, C", SET(0, C), 2},		{"SET 0, D", SET(0, D), 2},			{"SET 0, E", SET(0, E), 2},		{"SET 0, H", SET(0, H), 2},		{"SET 0, L", SET(0, L), 2},		{"SET 0, (HL)", SET(0, &HL), 4},	{"SET 0, A", SET(0, A), 2},		{"SET 1, B", SET(1, B), 2},			{"SET 1, C", SET(1, C), 2},			{"SET 1, D", SET(1, D), 2},			{"SET 1, E", SET(1, E), 2},		{"SET 1, H", SET(1, H), 2},			{"SET 1, L", SET(1, L), 2},		{"SET 1, (HL)", SET(1, &HL), 4},	{"SET 1, A", SET(1, A), 2}},
		{{"SET 2, B", SET(2, B), 2},	{"SET 2, C", SET(2, C), 2},		{"SET 2, D", SET(2, D), 2},			{"SET 2, E", SET(2, E), 2},		{"SET 2, H", SET(2, H), 2},		{"SET 2, L", SET(2, L), 2},		{"SET 2, (HL)", SET(2, &HL), 4},	{"SET 2, A", SET(2, A), 2},		{"SET 3, B", SET(3, B), 2},			{"SET 3, C", SET(3, C), 2},			{"SET 3, D", SET(3, D), 2},			{"SET 3, E", SET(3, E), 2},		{"SET 3, H", SET(3, H), 2},			{"SET 3, L", SET(3, L), 2},		{"SET 3, (HL)", SET(3, &HL), 4},	{"SET 3, A", SET(3, A), 2}},
		{{"SET 4, B", SET(4, B), 2},	{"SET 4, C", SET(4, C), 2},		{"SET 4, D", SET(4, D), 2},			{"SET 4, E", SET(4, E), 2},		{"SET 4, H", SET(4, H), 2},		{"SET 4, L", SET(4, L), 2},		{"SET 4, (HL)", SET(4, &HL), 4},	{"SET 4, A", SET(4, A), 2},		{"SET 5, B", SET(5, B), 2},			{"SET 5, C", SET(5, C), 2},			{"SET 5, D", SET(5, D), 2},			{"SET 5, E", SET(5, E), 2},		{"SET 5, H", SET(5, H), 2},			{"SET 5, L", SET(5, L), 2},		{"SET 5, (HL)", SET(5, &HL), 4},	{"SET 5, A", SET(5, A), 2}},
		{{"SET 6, B", SET(6, B), 2},	{"SET 6, C", SET(6, C), 2},		{"SET 6, D", SET(6, D), 2},			{"SET 6, E", SET(6, E), 2},		{"SET 6, H", SET(6, H), 2},		{"SET 6, L", SET(6, L), 2},		{"SET 6, (HL)", SET(6, &HL), 4},	{"SET 6, A", SET(6, A), 2},		{"SET 7, B", SET(7, B), 2},			{"SET 7, C", SET(7, C), 2},			{"SET 7, D", SET(7, D), 2},			{"SET 7, E", SET(7, E), 2},		{"SET 7, H", SET(7, H), 2},			{"SET 7, L", SET(7, L), 2},		{"SET 7, (HL)", SET(7, &HL), 4},	{"SET 7, A", SET(7, A), 2}}
	}};
}

bool CPU::isStopped() {
	return stopped;
}

void CPU::attachBus(Bus* bus) {
	this->bus = bus;
}

u8 CPU::fetch() {
	return bus->read(PC.value++);
}

u16 CPU::doubleFetch() {
	u8 first = fetch();
	u8 last = fetch();
	return last << 8 | first;
}

int CPU::step() {
	int cycles;
	u8 instruction = fetch();
	bool is16bit = false;
	if (instruction == 0xCB) {
		is16bit = true;
		instruction = fetch();
	}
	auto& instructionDetails = lookup[is16bit][hi(instruction)][lo(instruction)];
	cycles = instructionDetails.cycles;
	if (instructionDetails.name != "NOP")
	{
		cout << std::dec << PC.value - 1 << " (" << std::hex << PC.value - 1 << ") : " << instructionDetails.name << endl;
	}
	cycles += instructionDetails.fn();
	return cycles;
}

string CPU::nextInstruction() {
	return "";
	/*u8 instruction = fetch();
	if (instruction == 0xCB) {
		u8 instruction = fetch();
		return lookup_16Bit[hi(instruction)][lo(instruction)].name;
	}
	else {
		return lookup_8Bit[hi(instruction)][lo(instruction)].name;
	}*/
}

bool CPU::getFlag(FLAGS f) { return AF.low & f; }
void CPU::setFlag(FLAGS f, bool v) { (v) ? AF.low |= f : AF.low &= ~f; }
void CPU::clearFlags() { AF.low &= 0; }

op CPU::SUB() {
	return [&](u8* x, u8 y) {
		setFlag(N, true);
		int res = *x - y;
		int carrybits = *x ^ y ^ res;
		setFlag(C, (carrybits & 0x100) != 0);
		setFlag(H, (carrybits & 0x10) != 0);
		*x -= y;
		setFlag(Z, *x == 0);
	};
}
op CPU::ADC() {
	return [&](u8* x, u8 y) {
		int carry = getFlag(C);
		int res = *x + y + carry;
		clearFlags();
		setFlag(C, res > 0xFF);
		setFlag(H, ((*x & 0xF) + (y & 0xF) + carry > 0xF));
		*x += y + carry;
		setFlag(Z, *x == 0);
	};
}
op CPU::SBC() {
	return [&](u8* x, u8 y) {
		setFlag(N, true);
		int carry = getFlag(C);
		int res = *x - y - carry;
		setFlag(C, res < 0);
		setFlag(H, (*x & 0xF) - (y & 0xF) - carry < 0);
		*x -= y + getFlag(C);
		setFlag(Z, *x == 0);
	};
}
op CPU::AND() {
	return [&](u8* x, u8 y) {
		clearFlags();
		setFlag(H, true);
		*x &= y;
		setFlag(Z, *x == 0);
	};
}
op CPU::XOR() {
	return [&](u8* x, u8 y) {
		clearFlags();
		*x ^= y;
		setFlag(Z, *x == 0);
	};
}
op CPU::OR() {
	return [&](u8* x, u8 y) {
		clearFlags();
		*x |= y;
		setFlag(Z, *x == 0);
	};
}
op CPU::CP() {
	return [&](u8* x, u8 y) {
		setFlag(N, true);
		int res = *x - y;
		int carrybits = *x ^ y ^ res;
		setFlag(C, (carrybits & 0x100) != 0);
		setFlag(H, (carrybits & 0x10) != 0);
		setFlag(Z, (res & 0xFF) == 0);
	};
}

function<bool()> CPU::COND(FLAGS f, bool val) { return [&, f, val]() { return getFlag(f) == val; }; }

function<int()> CPU::NOP() { return []() { return 0; }; }

function<int()> CPU::LD(Register* reg) {
	return [&, reg]() {
		reg->value = doubleFetch();
		return 0;
	};
}

function<int()> CPU::LD(Register* addr, u8* reg) {
	return [&, addr, reg]() {
		bus->write(addr->value, *reg);
		return 0;
	};
}

function<int()> CPU::LD(Register* reg1, Register* reg2) {
	return [&, reg1, reg2]() {
		reg1->value = reg2->value;
		return 0;
	};
}

std::function<int()> CPU::LD(u8* reg, Register* addr) {
	return [&, reg, addr]() {
		*reg = bus->read(addr->value);
		return 0;
	};
}

function<int()> CPU::LD(u8* reg) {
	return [&, reg]() {
		*reg = fetch();
		return 0;
	};
}

function<int()> CPU::LD(u8* reg1, u8* reg2) {
	return [&, reg1, reg2]() {
		*reg1 = *reg2;
		return 0;
	};
}

function<int()> CPU::LDI(Register* addr, u8* reg) {
	return [&, addr, reg]() {
		bus->write(addr->value, *reg);
		addr->value++;
		return 0;
	};
}

function<int()> CPU::LDI(u8* reg, Register* addr) {
	return [&, reg, addr]() {
		*reg = bus->read(addr->value);
		addr->value++;
		return 0;
	};
}

function<int()> CPU::LDD(Register* addr, u8* reg) {
	return [&, addr, reg]() {
		bus->write(addr->value, *reg);
		addr->value--;
		return 0;
	};
}

function<int()> CPU::LDD(u8* reg, Register* addr) {
	return [&, reg, addr]() {
		*reg = bus->read(addr->value);
		addr->value--;
		return 0;
	};
}

function<int()> CPU::LDH(u8* reg) {
	return [&, reg]() {
		*reg = fetch() + 0xFF00;
		return 0;
	};
}

function<int()> CPU::LDH(u8* reg1, u8* reg2) {
	return [&, reg1, reg2]() {
		*reg1 = *reg2 + 0xFF00;
		return 0;
	};
}

function<int()> CPU::STH(u8* reg) {
	return [&, reg]() {
		bus->write(fetch() + 0xFF00, *reg);
		return 0;
	};
}

function<int()> CPU::STH(u8* reg1, u8* reg2) {
	return [&, reg1, reg2]() {
		bus->write(*reg1 + 0xFF00, *reg2);
		return 0;
	};
}

function<int()> CPU::LDA(u8* reg) {
	return [&, reg]() {
		*reg = bus->read(doubleFetch());
		return 0;
	};
}

function<int()> CPU::STA(u8* reg) {
	return [&, reg]() {
		bus->write(doubleFetch(), *reg);
		return 0;
	};
}

function<int()> CPU::ST(Register* reg) {
	return [&, reg]() {
		bus->write(reg->value, fetch());
		return 0;
	};
}

function<int()> CPU::STSP(Register* reg) {
	return [&, reg]() {
		u16 addr = doubleFetch();
		bus->write(addr, reg->value & 0xFF);
		bus->write(addr + 1, (reg->value & 0xFF00) >> 8);
		return 0;
	};
}

function<int()> CPU::LDHL(Register* reg1, Register* reg2) {
	return [&, reg1, reg2]() {
		s8 val = toSigned(fetch());
		reg1->value = reg2->value + val;
		clearFlags();
		if (((reg2->value ^ val ^ (reg2->value + val)) & 0x100) == 0x100)
			setFlag(C, true);
		if (((reg2->value ^ val ^ (reg2->value + val)) & 0x10) == 0x10)
			setFlag(H, true);
		return 0;
	};
}

function<int()> CPU::ADD() {
	return [&]() {
		u8 val = fetch();
		clearFlags();
		setFlag(H, checkHalfCarry(AF.high, val));
		setFlag(C, checkCarry(AF.high, val));
		AF.high += val;
		setFlag(Z, AF.high == 0);
		return 0;
	};
}

function<int()> CPU::ADD(u8* reg) {
	return [&, reg]() {
		u8 val = *reg;
		clearFlags();
		setFlag(H, checkHalfCarry(AF.high, val));
		setFlag(C, checkCarry(AF.high, val));
		AF.high += val;
		setFlag(Z, AF.high == 0);
		return 0;
	};
}

function<int()> CPU::ADD(u8* reg, Register* addr) {
	return [&, reg, addr]() {
		u8 val = bus->read(addr->value);
		clearFlags();
		setFlag(H, checkHalfCarry(*reg, val));
		setFlag(C, checkCarry(*reg, val));
		*reg += val;
		setFlag(Z, *reg == 0);
		return 0;
	};
}

function<int()> CPU::ADD(Register* reg) {
	return [&, reg]() {
		s8 val = toSigned(fetch());
		clearFlags();
		if (((reg->value ^ val ^ (reg->value + val)) & 0x100) == 0x100)
			setFlag(C, true);
		if (((reg->value ^ val ^ (reg->value + val)) & 0x10) == 0x10)
			setFlag(H, true);
		reg->value += val;
		setFlag(Z, reg->value == 0);
		return 0;
	};
}

function<int()> CPU::ADD(Register* reg1, Register* reg2) {
	return [&, reg1, reg2]() {
		int res = reg1->value + reg2->value;
		setFlag(N, false);
		setFlag(C, res & 0x10000);
		setFlag(H, (reg1->value ^ reg2->value ^ (res & 0xFFFF)) & 0x1000);
		reg1->value += reg2->value;
		return 0;
	};
}

function<int()> CPU::PUSH(Register* reg) {
	return [&, reg]() {
		bus->write(SP.value, reg->low);
		bus->write(SP.value + 1, reg->high);
		return 0;
	};
}

function<int()> CPU::POP(Register* reg) {
	return [&, reg]() {
		reg->low = bus->read(SP.value++);
		reg->high = bus->read(SP.value++);
		return 0;
	};
}

function<int()> CPU::OP(op f) {
	return [&, f]() {
		f(&(AF.high), fetch());
		return 0;
	};
}

function<int()> CPU::OP(u8* reg, op f) {
	return [&, reg, f]() {
		f(&(AF.high), *reg);
		return 0;
	};
}

function<int()> CPU::OP(Register* reg, op f) {
	return [&, reg, f]() {
		f(&(AF.high), bus->read(reg->value));
		return 0;
	};
}

function<int()> CPU::INC() {
	return [&]() {
		int val = bus->read(HL.value);
		int res = val + 1;
		bus->write(HL.value, res);
		setFlag(N, false);
		setFlag(Z, (res & 0xFF) == 0);
		setFlag(H, (val & 0xF) + 1 > 0xF);
		return 0;
	};
}

function<int()> CPU::INC(u8* reg) {
	return [&, reg]() {
		int val = *reg;
		(*reg)++;
		setFlag(N, false);
		setFlag(Z, *reg == 0);
		setFlag(H, (val & 0xF) + 1 > 0xF);
		return 0;
	};
}

function<int()> CPU::INC(Register* reg) {
	return [&, reg]() {
		(reg->value)++;
		return 0;
	};
}

function<int()> CPU::DEC() {
	return [&]() {
		int val = bus->read(HL.value);
		int res = val - 1;
		bus->write(HL.value, res);
		setFlag(N, true);
		setFlag(Z, (res & 0xFF) == 0);
		setFlag(H, (val & 0xF) - 1 < 0);
		return 0;
	};
}

function<int()> CPU::DEC(u8* reg) {
	return [&, reg]() {
		int val = *reg;
		(*reg)--;
		setFlag(N, true);
		setFlag(Z, *reg == 0);
		setFlag(H, (val & 0xF) - 1 < 0);
		return 0;
	};
}

function<int()> CPU::DEC(Register* reg) {
	return [&, reg]() {
		(reg->value)--;
		return 0;
	};
}

function<int()> CPU::JR(function<bool()> cond) {
	return [&, cond]() {
		s8 offset = toSigned(fetch());
		bool jr = cond();
		if (jr) {
			PC.value += offset;
		}
		return jr;
	};
}

function<int()> CPU::JP(function<bool()> cond) {
	return [&, cond]() {
		u16 addr = doubleFetch();
		bool jr = cond();
		if (jr) {
			PC.value = addr;
		}
		return jr;
	};
}

function<int()> CPU::JP(Register* reg) {
	return [&, reg]() {
		PC.value = reg->value;
		return 0;
	};
}

function<int()> CPU::CALL(function<bool()> cond) {
	return [&, cond]() {
		u16 addr = doubleFetch();
		bool jr = cond();
		if (jr) {
			bus->write(--SP.value, PC.high);
			bus->write(--SP.value, PC.low);
			PC.value = addr;
		}
		return jr * 3;
	};
}

function<int()> CPU::RST(int x) {
	return [&, x]() {
		bus->write(--SP.value, PC.high);
		bus->write(--SP.value, PC.low);
		PC.value = x * 0x8;
		return 0;
	};
}

function<int()> CPU::RET(function<bool()> cond) {
	return [&, cond]() {
		bool jr = cond();
		if (jr) {
			PC.low = bus->read(SP.value++);
			PC.high = bus->read(SP.value++);
		}
		return jr * 3;
	};
}

function<int()> CPU::RET() {
	return [&]() {
		PC.low = bus->read(SP.value++);
		PC.high = bus->read(SP.value++);
		return 0;
	};
}

function<int()> CPU::RETI() {
	return [&]() {
		RET()();
		EI()();
		return 0;
	};
}

function<int()> CPU::CPL(u8* reg) {
	return [&, reg]() {
		*reg = ~(*reg);
		setFlag(N, true);
		setFlag(H, true);
		return 0;
	};
}

function<int()> CPU::RL(u8* reg) {
	return [&, reg]() {
		u8 top = *reg >> 7;
		*reg = (*reg << 1) | getFlag(C);
		clearFlags();
		setFlag(C, top);
		setFlag(Z, *reg == 0);
		return 0;
	};
}

function<int()> CPU::RL(Register* reg) {
	return [&, reg]() {
		u16 addr = reg->value;
		u8 val = bus->read(addr);
		u8 top = val >> 7;
		u8 res = (val << 1) | getFlag(C);
		bus->write(addr, res);
		clearFlags();
		setFlag(Z, res == 0);
		setFlag(C, top);
		return 0;
	};
}

function<int()> CPU::RLC(u8* reg) {
	return [&, reg]() {
		setFlag(C, *reg >> 7);
		*reg = (*reg << 1) | getFlag(C);
		clearFlags();
		setFlag(Z, *reg == 0);
		return 0;
	};
}

function<int()> CPU::RLC(Register* reg) {
	return [&, reg]() {
		u16 addr = reg->value;
		u8 val = bus->read(addr);
		u8 res = (val << 1) | getFlag(C);
		clearFlags();
		setFlag(Z, res == 0);
		setFlag(C, val >> 7);
		bus->write(addr, res);
		return 0;
	};
}

function<int()> CPU::RR(u8* reg) {
	return [&, reg]() {
		u8 bottom = *reg | 0x1;
		*reg = (*reg >> 1) | (getFlag(C) << 7);
		clearFlags();
		setFlag(Z, *reg == 0);
		setFlag(C, bottom);
		return 0;
	};
}

function<int()> CPU::RR(Register* reg) {
	return [&, reg]() {
		u16 addr = reg->value;
		u8 val = bus->read(addr);
		u8 bottom = val | 0x1;
		u8 res = (val >> 1) | (getFlag(C) << 7);
		bus->write(addr, res);
		clearFlags();
		setFlag(Z, res == 0);
		setFlag(C, bottom);
		return 0;
	};
}

function<int()> CPU::RRC(u8* reg) {
	return [&, reg]() {
		clearFlags();
		setFlag(C, *reg | 0x1);
		*reg = (*reg >> 1) | (getFlag(C) << 7);
		setFlag(Z, *reg == 0);
		return 0;
	};
}

function<int()> CPU::RRC(Register* reg) {
	return [&, reg]() {
		clearFlags();
		u16 addr = reg->value;
		u8 val = bus->read(addr);
		setFlag(C, val | 0x1);
		u8 res = (val >> 1) | (getFlag(C) << 7);
		bus->write(addr, res);
		setFlag(Z, res == 0);
		return 0;
	};
}

function<int()> CPU::RLCA() { return RLC(&(AF.high)); }
function<int()> CPU::RLA() { return RL(&(AF.high)); }
function<int()> CPU::RRCA() { return RRC(&(AF.high)); }
function<int()> CPU::RRA() { return RR(&(AF.high)); }

function<int()> CPU::SLA(u8* reg) {
	return [&, reg]() {
		clearFlags();
		setFlag(C, *reg >> 7);
		*reg = (*reg << 1) & 0xFE;
		setFlag(Z, *reg == 0);
		return 0;
	};
}

function<int()> CPU::SLA(Register* reg) {
	return [&, reg]() {
		clearFlags();
		u16 addr = reg->value;
		u8 val = bus->read(addr);
		setFlag(C, val >> 7);
		u8 res = (val << 1) & 0xFE;
		bus->write(addr, res);
		setFlag(Z, res == 0);
		return 0;
	};
}

function<int()> CPU::SRA(u8* reg) {
	return [&, reg]() {
		clearFlags();
		setFlag(C, *reg | 0x1);
		*reg = (*reg >> 1) | (*reg & (1 << 7));
		setFlag(Z, *reg == 0);
		return 0;
	};
}

function<int()> CPU::SRA(Register* reg) {
	return [&, reg]() {
		clearFlags();
		u16 addr = reg->value;
		u8 val = bus->read(addr);
		setFlag(C, val | 0x1);
		u8 res = (val >> 1) | (val & (1 << 7));
		bus->write(addr, res);
		setFlag(Z, res == 0);
		return 0;
	};
}

function<int()> CPU::SRL(u8* reg) {
	return [&, reg]() {
		clearFlags();
		setFlag(C, *reg | 0x1);
		*reg = (*reg >> 1) & 0x7F;
		setFlag(Z, *reg == 0);
		return 0;
	};
}

function<int()> CPU::SRL(Register* reg) {
	return [&, reg]() {
		clearFlags();
		u16 addr = reg->value;
		u8 val = bus->read(addr);
		setFlag(C, val | 0x1);
		u8 res = (val >> 1) & 0x7F;
		bus->write(addr, res);
		setFlag(Z, res == 0);
		return 0;
	};
}

function<int()> CPU::SWAP(u8* reg) {
	return [&, reg]() {
		u8 res = (*reg >> 4) | (*reg << 4);
		*reg = res;
		clearFlags();
		setFlag(Z, res == 0);
		return 0;
	};
}

function<int()> CPU::SWAP(Register* reg) {
	return [&, reg]() {
		u16 addr = reg->value;
		u8 val = bus->read(addr);
		u8 res = (val >> 4) | (val << 4);
		bus->write(addr, res);
		clearFlags();
		setFlag(Z, res == 0);
		return 0;
	};
}

function<int()> CPU::BIT(u8 bitNumber, u8* reg) {
	return [&, bitNumber, reg]() {
		setFlag(Z, ((*reg >> bitNumber) & 0x1) == 0);
		setFlag(N, false);
		setFlag(H, true);
		return 0;
	};
}

function<int()> CPU::BIT(u8 bitNumber, Register* reg) {
	return [&, bitNumber, reg]() {
		u16 addr = reg->value;
		u8 val = bus->read(addr);
		setFlag(Z, ((val >> bitNumber) & 0x1) == 0);
		setFlag(N, false);
		setFlag(H, true);
		return 0;
	};
}

function<int()> CPU::SET(u8 bitNumber, u8* reg) {
	return [&, bitNumber, reg]() {
		*reg |= 0x1 << bitNumber;
		return 0;
	};
}

function<int()> CPU::SET(u8 bitNumber, Register* reg) {
	return [&, bitNumber, reg]() {
		u16 addr = reg->value;
		u8 val = bus->read(addr);
		bus->write(addr, val | (0x1 << bitNumber));
		return 0;
	};
}

function<int()> CPU::RES(u8 bitNumber, u8* reg) {
	return [&, bitNumber, reg]() {
		*reg |= ~(0x1 << bitNumber);
		return 0;
	};
}

function<int()> CPU::RES(u8 bitNumber, Register* reg) {
	return [&, bitNumber, reg]() {
		u16 addr = reg->value;
		u8 val = bus->read(addr);
		bus->write(addr, val | ~(0x1 << bitNumber));
		return 0;
	};
}

function<int()> CPU::CCF() {
	return [&]() {
		setFlag(C, !getFlag(C));
		setFlag(N, false);
		setFlag(H, false);
		return 0;
	};
}

function<int()> CPU::SCF() {
	return [&]() {
		setFlag(C, true);
		setFlag(N, false);
		setFlag(H, false);
		return 0;
	};
}

function<int()> CPU::DI() {
	return [&]() {
		interuptsEnabled = false;
		return 0;
	};
}

function<int()> CPU::EI() {
	return [&]() {
		interuptsEnabled = true;
		return 0;
	};
}

function<int()> CPU::HALT() {
	return [&]() {
		stopped = true;
		return 0;
	};
}

function<int()> CPU::STOP() {
	return [&]() {
		stopped = true;
		return 0;
	};
}

function<int()> CPU::DAA() {
	return [&]() {
		// Details: https://ehaskins.com/2018-01-30%20Z80%20DAA/
		// Implemtation: https://github.com/drhelius/Gearboy/blob/master/src/opcodes.cpp

		u8 a = AF.high;

		if (!getFlag(N)) {
			if (getFlag(H) || ((a & 0xF) > 9)) a += 0x06;
			if (getFlag(C) || (a > 0x9F)) a += 0x60;
		}
		else {
			if (getFlag(H)) a = (a - 6) & 0xFF;
			if (getFlag(C)) a -= 0x60;
		}

		setFlag(H, !getFlag(H));
		setFlag(Z, !getFlag(Z));

		if ((a & 0x100) == 0x100) setFlag(C, true);

		a &= 0xFF;
		setFlag(Z, a == 0);
		AF.high = a;

		return 0;
	};
}

function<int()> CPU::XXX() {
	return []() {
		throw new exception("INVALID OPCODE");
		return 0;
	};
}
