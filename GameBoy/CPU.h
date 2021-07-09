#pragma once

#include <string>
#include <vector>
#include <functional>

#include "definitions.h"
#include "Bus.h"

class CPU;

typedef std::function<void(u8*, u8)> op;

struct Instruction {
    std::string name;
    std::function<int()> fn;
    int cycles;
};

union Register
{
    u16 value;
    struct
    {
        u8 low;
        u8 high;
    };
};

enum FLAGS
{
    U1 = (1 << 0),	// Unused
    U2 = (1 << 1),	// Unused
    U3 = (1 << 2),	// Unused
    U4 = (1 << 3),	// Unused
    C = (1 << 4),	// Carry Flag
    H = (1 << 5),	// Half Carry Flag (BCD)
    N = (1 << 6),	// Add/Sub-Flag (BCD)
    Z = (1 << 7),	// Zero
};

class CPU {
private:
    Register AF = { 0x11b0 };
    Register BC = { 0x0013 };
    Register DE = { 0x00D8 };
    Register HL = { 0x014D };
    Register SP = { 0xFFFE };
    Register PC = { 0x0100 };

    bool interuptsEnabled = true;
    bool stopped = false;

    std::vector<std::vector<std::vector<Instruction>>> lookup;

    Bus* bus = nullptr;
    u8 fetch();
    u16 doubleFetch();

    u8* getRegister(u8 i);
    bool getFlag(FLAGS f);
    void setFlag(FLAGS f, bool v);
    void clearFlags();

    op SUB();
    op ADC();
    op SBC();
    op AND();
    op XOR();
    op OR();
    op CP();

    std::function<bool()> COND(FLAGS f, bool val = true);

    std::function<int()> NOP();
    std::function<int()> LD(Register* reg);
    std::function<int()> LD(Register* addr, u8* reg);
    std::function<int()> LD(Register* reg1, Register* reg2);
    std::function<int()> LD(u8* reg, Register* addr);
    std::function<int()> LD(u8* reg);
    std::function<int()> LD(u8* reg1, u8* reg2);
    std::function<int()> LDI(Register* addr, u8* reg);
    std::function<int()> LDI(u8* reg, Register* addr);
    std::function<int()> LDD(Register* addr, u8* reg);
    std::function<int()> LDD(u8* reg, Register* addr);
    std::function<int()> LDH(u8* reg);
    std::function<int()> LDH(u8* reg1, u8* reg2);
    std::function<int()> STH(u8* reg);
    std::function<int()> STH(u8* reg1, u8* reg2);
    std::function<int()> LDA(u8* reg);
    std::function<int()> STA(u8* reg);
    std::function<int()> ST(Register* reg);
    std::function<int()> STSP(Register* reg);
    std::function<int()> LDHL(Register* reg1, Register* reg2);
    std::function<int()> ADD();
    std::function<int()> ADD(u8* reg);
    std::function<int()> ADD(u8* reg, Register* addr);
    std::function<int()> ADD(Register* reg);
    std::function<int()> ADD(Register* reg1, Register* reg2);
    std::function<int()> PUSH(Register* reg);
    std::function<int()> POP();
    std::function<int()> POP(Register* reg);
    std::function<int()> OP(op f);
    std::function<int()> OP(u8* reg, op f);
    std::function<int()> OP(Register* reg, op f);
    std::function<int()> INC();
    std::function<int()> INC(u8* reg);
    std::function<int()> INC(Register* reg);
    std::function<int()> DEC();
    std::function<int()> DEC(u8* reg);
    std::function<int()> DEC(Register* reg);
    std::function<int()> JR(std::function<bool()> cond);
    std::function<int()> JP(std::function<bool()> cond);
    std::function<int()> JP(Register* reg);
    std::function<int()> CALL(std::function<bool()> cond);
    std::function<int()> RET(std::function<bool()> cond);
    std::function<int()> RET();
    std::function<int()> RETI();
    std::function<int()> DAA();
    std::function<int()> CPL(u8* reg);
    std::function<int()> RLCA();
    std::function<int()> RLA();
    std::function<int()> RRCA();
    std::function<int()> RRA();
    std::function<int()> RLC(u8* reg, bool isA = false);
    std::function<int()> RLC(Register* reg);
    std::function<int()> RL(u8* reg, bool isA = false);
    std::function<int()> RL(Register* reg);
    std::function<int()> RRC(u8* reg, bool isA = false);
    std::function<int()> RRC(Register* reg);
    std::function<int()> RR(u8* reg, bool isA = false);
    std::function<int()> RR(Register* reg);
    std::function<int()> SLA(u8* reg);
    std::function<int()> SLA(Register* reg);
    std::function<int()> SRA(u8* reg);
    std::function<int()> SRA(Register* reg);
    std::function<int()> SRL(u8* reg);
    std::function<int()> SRL(Register* reg);
    std::function<int()> SWAP(u8* reg);
    std::function<int()> SWAP(Register* reg);
    std::function<int()> BIT(u8 bitNumber, u8* reg);
    std::function<int()> BIT(u8 bitNumber, Register* reg);
    std::function<int()> SET(u8 bitNumber, u8* reg);
    std::function<int()> SET(u8 bitNumber, Register* reg);
    std::function<int()> RES(u8 bitNumber, u8* reg);
    std::function<int()> RES(u8 bitNumber, Register* reg);
    std::function<int()> CCF();
    std::function<int()> SCF();
    std::function<int()> HALT();
    std::function<int()> STOP();
    std::function<int()> DI();
    std::function<int()> EI();
    std::function<int()> RST(int x);
    std::function<int()> XXX();

public:
	CPU();
	int step();
    std::string nextInstruction();
    void attachBus(Bus* bus);
    bool isStopped();
    void printState();
};
