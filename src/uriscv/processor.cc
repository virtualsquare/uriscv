#include "uriscv/processor.h"
#include "uriscv/bus.h"
#include "uriscv/const.h"
#include "uriscv/utility.h"
#include <cstdint>
#include <iostream>

#define I_TYPE 0x13
#define ADDI 0x0
#define SLLI 0x1
#define SLTI 0x2
#define SLTIU 0x3
#define XORI 0x4
#define SRI 0x5
#define SRLI 0x00
#define SRAI 0x20
#define ORI 0x6
#define ANDI 0x7

#define I2_TYPE 0x73
#define CSRRW 0x1
#define CSRRS 0x2
#define CSRRC 0x3
#define CSRRWI 0x5
#define CSRRSI 0x6
#define CSRRCI 0x7

#define R_TYPE 0x33
#define ADD 0x0
#define ADD_func7 0x0
#define SUB 0x0
#define SUB_func7 0x20
#define SLL 0x1
#define SLT 0x2
#define SLTU 0x3
#define XOR 0x4
#define SRL 0x5
#define SRL_func7 0x0
#define SRA 0x5
#define SRA_func7 0x20
#define OR 0x6
#define AND 0x7

#define B_TYPE 0x63
#define BEQ 0x0
#define BNE 0x1
#define BLT 0x4
#define BGE 0x5
#define BLTU 0x6
#define BGEU 0x7

#define JAL 0x6F
#define JALR 0x67

#define OPCODE(instr) (instr & 0x7F)
#define RD(instr) ((instr >> 7) & 0x1F)
#define FUNCT3(instr) ((instr >> 12) & 0x7)
#define RS1(instr) ((instr >> 15) & 0x1F)
#define RS2(instr) ((instr >> 20) & 0x1F)
#define FUNCT7(instr) ((instr >> 25) & 0x7F)

#define I_IMM(instr) (instr >> 20)
#define B_IMM(instr)                                                           \
  (((instr & 0xF00) >> 7) | ((instr & 0x80) << 4) |                            \
   ((instr & 0x80000000) >> 19) | ((instr & 0x7E000000) >> 20))
#define JAL_OFFSET(instr) (instr >> 20)

Processor::Processor(Config *config) {
  this->config = config;
  this->bus = new Bus(this->config);
}

void Processor::Init(Word pc, Word sp) {
  this->pc = pc;
  this->sp = sp;
  for (uint8_t i = 0; i < NUM_REGS; i++) {
    gregs[i] = 0;
  }
}

// physical pc, not virtual
Word Processor::Fetch(Word pc) {
  Word instr;
  if (this->bus->Read(pc, &instr))
    // TODO: signal error
    return 0x0;

  return instr;
}

Word Processor::Fetch() { return this->Fetch(this->pc); }

bool Processor::Excecute(Word instr) {
  printf("Instr %08X\n", instr);
  Utility::printb(sizeof(instr), &instr);
  uint8_t opcode = OPCODE(instr);
  uint8_t func3 = FUNCT3(instr);
  printf("opcode %08X\n", opcode);
  switch (opcode) {
  case R_TYPE: {
    printf("R-type\n");
    switch (func3) {
    case ADD:
      std::cout << "\tADD\n";
      int8_t rs1 = RS1(instr);
      int8_t rs2 = RS2(instr);
      uint8_t rd = RD(instr);
      this->GRegWrite(rd, rs1 + rs2);
      break;
    }
    this->pc += WORDLEN;
    break;
  }
  case I_TYPE: {
    printf("I-type\n");
    switch (func3) {
    case ADDI:
      std::cout << "\tADDI\n";
      int8_t rs1 = RS1(instr);
      int16_t imm = I_IMM(instr);
      uint8_t rd = RD(instr);
      this->GRegWrite(rd, rs1 + imm);
      break;
    }
    this->pc += WORDLEN;
    break;
  }
  case I2_TYPE: {
    printf("I2-type\n");
    uint16_t imm = I_IMM(instr);
    int8_t rs1 = RS1(instr);
    uint8_t rd = RD(instr);
    Utility::printb(sizeof(imm), &imm);
    switch (func3) {
    case CSRRW: {
      std::cout << "\tCSRRW\n";
      if (rd != 0x0)
        this->GRegWrite(rd, this->CSRRead(imm));
      if (rs1 != 0x0)
        this->CSRWrite(imm, this->GRegRead(rs1));
      break;
    }
    case CSRRS: {
      std::cout << "\tCSRRS\n";
      this->GRegWrite(rd, this->CSRRead(imm));
      this->CSRWrite(imm, this->CSRRead(imm) | this->GRegRead(rs1));
      break;
    }
    case CSRRC: {
      std::cout << "\tCSRRC\n";
      this->GRegWrite(rd, this->CSRRead(imm));
      if (rs1 != 0x0)
        this->CSRWrite(imm, this->CSRRead(imm) & ~this->GRegRead(rs1));
      break;
    }
    case CSRRWI: {
      std::cout << "\tCSRRWI\n";
      this->GRegWrite(rd, this->CSRRead(imm));
      this->CSRWrite(imm, this->CSRRead(imm) & ~rs1);
      break;
    }
    case CSRRSI: {
      std::cout << "\tCSRRSI\n";
      this->GRegWrite(rd, this->CSRRead(imm));
      this->CSRWrite(imm, this->CSRRead(imm) | rs1);
      break;
    }
    case CSRRCI: {
      std::cout << "\tCSRRCI\n";
      this->GRegWrite(rd, this->CSRRead(imm));
      this->CSRWrite(imm, this->CSRRead(imm) & ~rs1);
      break;
    }
    }
    this->pc += WORDLEN;
    break;
  }
  case B_TYPE: {
    printf("B-type\n");
    // int8_t rs1 = RS1(instr);
    // int8_t rs2 = RS2(instr);
    int16_t imm = B_IMM(instr);
    printf("Imm %d\n", imm);
    switch (func3) {
    case BEQ: {
      std::cout << "\tBEQ\n";
      break;
    }
    case BNE: {
      std::cout << "\tBNE\n";
      break;
    }
    case BLT: {
      std::cout << "\tBLT\n";
      break;
    }
    case BGE: {
      std::cout << "\tBGE\n";
      break;
    }
    case BLTU: {
      std::cout << "\tBLTU\n";
      break;
    }
    case BGEU: {
      std::cout << "\tBGEU\n";
      break;
    }
    }
    break;
  }
  case JAL: {
    printf("J-type\n\tJAL\n");
    uint8_t rd = RD(instr);
    uint32_t offset = JAL_OFFSET(instr);
    printf("\trd %08X\n", rd);
    printf("\toffset %08X\n", offset);
    this->GRegWrite(rd, this->pc + WORDLEN);
    this->pc += offset;
    break;
  }
  case JALR: {
    printf("J-type\n\tJALR\n");
    break;
  }
  }

  return true;
}

Word Processor::GRegRead(Word reg) { return gregs[reg]; }
void Processor::GRegWrite(Word reg, Word value) {
  // reg 0 in hardwired to 0
  if (reg != 0)
    gregs[reg] = value;
}
Word Processor::CSRRead(Word reg) {
  // TODO : check perm
  return csrs[reg].value;
}
void Processor::CSRWrite(Word reg, Word value) {
  // TODO : check perm
  csrs[reg].value = value;
}

void Processor::initCSR() {
  this->csrs[CYCLE].perm = RRR;
  this->csrs[CYCLEH].perm = NNW;
  this->csrs[MCYCLE].perm = RRR;
  this->csrs[MCYCLEH].perm = RRR;

  this->csrs[TIME].perm = RRR;
  this->csrs[TIMEH].perm = RRR;

  this->csrs[INSTRET].perm = RRR;
  this->csrs[MINSTRET].perm = NNW;
  this->csrs[INSTRETH].perm = RRR;
  this->csrs[MINSTRETH].perm = NNW;

  this->csrs[0x000].perm = WWW;
  this->csrs[0x100].perm = NWW;
  this->csrs[0x300].perm = NNW;

  this->csrs[0x004].perm = WWW;
  this->csrs[0x104].perm = NWW;
  this->csrs[0x304].perm = NNW;

  this->csrs[0x005].perm = WWW;
  this->csrs[0x105].perm = NWW;
  this->csrs[0x305].perm = NNW;

  this->csrs[0x102].perm = NWW;
  this->csrs[0x302].perm = NNW;

  this->csrs[0x103].perm = NWW;
  this->csrs[0x303].perm = NNW;

  this->csrs[0x106].perm = NWW;
  this->csrs[0x306].perm = NNW;

  for (Word i = 0; i < 5; i++) {
    this->csrs[0x040 + i].perm = WWW;
    this->csrs[0x140 + i].perm = NWW;
    this->csrs[0x340 + i].perm = NNW;
  }

  this->csrs[0x180].perm = NWW;

  this->csrs[0x301].perm = NNW;
  this->csrs[0xF11].perm = NNR;
  this->csrs[0xF12].perm = NNR;
  this->csrs[0xF13].perm = NNR;
  this->csrs[0xF14].perm = NNR;

  this->csrs[0x001].perm = WWW;
  this->csrs[0x002].perm = WWW;
  this->csrs[0x003].perm = WWW;

  for (Word i = 0; i < 0x33F - 0x323 + 1; i++) {
    this->csrs[0x323 + i].perm = NNW;
    this->csrs[0xC03 + i].perm = RRR;
    this->csrs[0xB03 + i].perm = NNW;
    this->csrs[0xC83 + i].perm = RRR;
    this->csrs[0xB83 + i].perm = NNW;
  }

  this->csrs[0x3A0].perm = NNW;
  this->csrs[0x3A1].perm = NNW;
  this->csrs[0x3A2].perm = NNW;
  this->csrs[0x3A3].perm = NNW;

  for (Word i = 0; i < 0xF; i++) {
    this->csrs[0x3B0 + i].perm = NNW;
  }

  for (Word i = 0; i < 0x7B2 - 0x7A0 + 1; i++) {
    this->csrs[0x7A0 + i].perm = NNW;
  }
}
