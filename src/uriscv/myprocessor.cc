// https://www.cs.cornell.edu/courses/cs3410/2019sp/schedule/

#include "uriscv/config.h"
#include "uriscv/const.h"
#include "uriscv/hart.h"
#include "uriscv/mybus.h"
#include "uriscv/processor.h"
#include "uriscv/utility.h"
#include <assert.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#define DEBUG 0

#define OP_L 0x3
#define OP_LB 0x0
#define OP_LH 0x1
#define OP_LW 0x2
#define OP_LBU 0x4
#define OP_LHU 0x5

#define I_TYPE 0x13
#define OP_ADDI 0x0
#define OP_SLLI 0x1
#define OP_SLTI 0x2
#define OP_SLTIU 0x3
#define OP_XORI 0x4
#define OP_SR 0x05
#define OP_SRLI_func7 0x00
#define OP_SRAI_func7 0x20
#define OP_ORI 0x6
#define OP_ANDI 0x7

#define I2_TYPE 0x73
#define OP_ECALL_EBREAK 0x0
#define ECALL_IMM 0x0
#define EBREAK_IMM 0x1
#define OP_CSRRW 0x1
#define OP_CSRRS 0x2
#define OP_CSRRC 0x3
#define OP_CSRRWI 0x5
#define OP_CSRRSI 0x6
#define OP_CSRRCI 0x7
#define R_TYPE 0x33
#define OP_ADD 0x0
#define OP_ADD_func7 0x0
#define OP_SUB 0x0
#define OP_SUB_func7 0x20
#define OP_SLL 0x1
#define OP_SLT 0x2
#define OP_SLTU 0x3
#define OP_XOR 0x4
#define OP_SRL 0x5
#define OP_SRL_func7 0x0
#define OP_SRA 0x5
#define OP_SRA_func7 0x20
#define OP_OR 0x6
#define OP_AND 0x7

#define B_TYPE 0x63
#define OP_BEQ 0x0
#define OP_BNE 0x1
#define OP_BLT 0x4
#define OP_BGE 0x5
#define OP_BLTU 0x6
#define OP_BGEU 0x7

#define S_TYPE 0x23
#define OP_SB 0x0
#define OP_SH 0x1
#define OP_SW 0x2

#define OP_AUIPC 0x17
#define OP_LUI 0x37
#define OP_JAL 0x6F
#define OP_JALR 0x67

#define OP_FENCE 0xF
#define OP_FENCE_TSO 0x8330000F
#define OP_PAUSE 0x100000F
#define FENCE_PERM(instr) ((instr >> 20) & 0xFF)
#define FENCE_SUCC(instr) ((instr >> 20) & 0xF)
#define FENCE_PRED(instr) ((instr >> 24) & 0xF)
#define FENCE_PI(instr) (FENCE_PRED(instr) & 0x1)
#define FENCE_PO(instr) ((FENCE_PRED(instr) >> 1) & 0x1)
#define FENCE_PR(instr) ((FENCE_PRED(instr) >> 2) & 0x1)
#define FENCE_PW(instr) ((FENCE_PRED(instr) >> 3) & 0x1)
#define FENCE_SI(instr) (FENCE_SUCC(instr) & 0x1)
#define FENCE_SO(instr) ((FENCE_SUCC(instr) >> 1) & 0x1)
#define FENCE_SR(instr) ((FENCE_SUCC(instr) >> 2) & 0x1)
#define FENCE_SW(instr) ((FENCE_SUCC(instr) >> 3) & 0x1)

#define OPCODE(instr) (instr & 0x7F)
#define RD(instr) ((instr >> 7) & 0x1F)
#define FUNC3(instr) ((instr >> 12) & 0x7)
#define RS1(instr) ((instr >> 15) & 0x1F)
#define RS2(instr) ((instr >> 20) & 0x1F)
#define FUNC7(instr) ((instr >> 25) & 0x7F)

#define I_IMM_SIZE 12
#define I_IMM(instr) (instr >> 20)
#define B_IMM(instr)                                                           \
  (((instr & 0xF00) >> 7) | ((instr & 0x80) << 4) |                            \
   ((instr & 0x80000000) >> 19) | ((instr & 0x7E000000) >> 20))
#define U_IMM_SIZE 20
#define U_IMM(instr) (instr >> 12)
#define S_IMM_SIZE 12
#define S_IMM(instr) (RD(instr) | (FUNC7(instr) << 5))
#define J_IMM_SIZE 20
#define J_IMM(instr)                                                           \
  (((instr >> 24) & 0xFF) | (((instr >> 23) & 0x1) << 8) |                     \
   (((instr >> 13) & 0x1FF) << 9) | ((instr >> 12)))

#define SIGN_BIT(value, bits) (((value) & (1 << (bits - 1))) >> (bits - 1))
#define SIGN_EXTENSION(value, bits)                                            \
  (SIGN_BIT(value, bits) ? (((value) | (((1 << (32 - bits)) - 1)) << bits))    \
                         : value)

MyProcessor::MyProcessor(Config *config, MyBus *bus) : id(0) {
  this->config = config;
  this->bus = bus;
}
MyProcessor::MyProcessor(const MachineConfig *config, Word cpuId,
                         Machine *machine, SystemBus *bus)
    : id(cpuId) {}

void MyProcessor::Init(Word nharts, Word pc, Word sp) {
  assert(nharts <= NUM_HARTS);
  this->selHart = 0;
  this->harts = (Hart **)malloc(sizeof(Hart) * nharts);
  for (uint8_t i = 0; i < nharts; i++) {
    this->harts[i] = new Hart(this->bus);
    this->harts[i]->Init(i, pc, sp);
  }
  this->old_mode = this->mode = MODE_MACHINE;
}

Hart *MyProcessor::GetCurrentHart() { return this->harts[this->selHart]; }

void MyProcessor::SetPC(Word value) { this->GetCurrentHart()->SetPC(value); }
void MyProcessor::SetSP(Word value) { this->GetCurrentHart()->SetSP(value); }
Word MyProcessor::IncrementPC(Word value) {
  return this->GetCurrentHart()->IncrementPC(value);
}
Word MyProcessor::IncrementSP(Word value) {
  return this->GetCurrentHart()->IncrementSP(value);
}
Word MyProcessor::GetPC() { return this->GetCurrentHart()->GetPC(); }
Word MyProcessor::GetSP() { return this->GetCurrentHart()->GetSP(); }

// physical pc, not virtual
exception_t MyProcessor::Fetch(Word pc, Word *instr) {
  return this->GetCurrentHart()->Fetch(pc, instr);
}
exception_t MyProcessor::Fetch(Word *instr) {
  return this->GetCurrentHart()->Fetch(instr);
}

exception_t MyProcessor::Excecute(Word instr) {
  printf("[%08X] %08X | ", this->GetPC(), instr);
  Utility::printb(sizeof(instr), &instr);
  exception_t e = EXC_OK;
  uint8_t opcode = OPCODE(instr);
  uint8_t func3 = FUNC3(instr);
  // printf("opcode %08X\n", opcode);
  switch (opcode) {
  case OP_L: {
    printf("\tI-type | ");
    int8_t rd = RD(instr);
    int8_t rs1 = RS1(instr);
    int16_t imm = I_IMM(instr);
    imm = SIGN_EXTENSION(imm, I_IMM_SIZE);
    Word read = 0;
    switch (func3) {
    case OP_LB: {
      printf("OP_LB\n");
      e = this->bus->Read((SWord)this->GRegRead(rs1) + (SWord)imm, &read);
      // just 8 bits
      read = SIGN_EXTENSION(read & 0xFF, 8);
      this->GRegWrite(rd, read);
      this->IncrementPC(WORDLEN);
      break;
    }
    case OP_LH: {
      printf("OP_LBH\n");
      e = this->bus->Read((SWord)this->GRegRead(rs1) + (SWord)imm, &read);
      // just 16 bits
      read = SIGN_EXTENSION(read & 0xFFFF, 16);
      this->GRegWrite(rd, read);
      this->IncrementPC(WORDLEN);
      break;
    }
    case OP_LW: {
      e = this->bus->Read((SWord)this->GRegRead(rs1) + (SWord)imm, &read);
      printf("OP_LW %d,%d(%x),%d\n", rd, rs1, this->GRegRead(rs1), (Word)imm);
      printf("read %x\n", read);
      this->GRegWrite(rd, read);
      this->IncrementPC(WORDLEN);
      break;
    }
    case OP_LBU: {
      printf("OP_LBU %d,%d(%x),%x\n", rd, rs1, this->GRegRead(rs1), imm);
      e = this->bus->Read((Word)this->GRegRead(rs1) + (Word)imm, &read);
      // just 8 bits
      read = read & 0xFF;
      this->GRegWrite(rd, read);
      this->IncrementPC(WORDLEN);
      break;
    }
    case OP_LHU: {
      printf("OP_LHU\n");
      e = this->bus->Read((Word)this->GRegRead(rs1) + (Word)imm, &read);
      // just 16 bits
      read = read & 0xFFFF;
      this->GRegWrite(rd, read);
      this->IncrementPC(WORDLEN);
      break;
    }
    default: {
      return EXC_ILL_INSTR;
    }
    }
    break;
  }
  case R_TYPE: {
    printf("\tR-type | ");
    int8_t rs1 = RS1(instr);
    int8_t rs2 = RS2(instr);
    uint8_t rd = RD(instr);
    uint8_t func7 = FUNC7(instr);
    switch (func3) {
    case OP_ADD: {
      switch (func7) {
      case OP_ADD_func7: {
        printf("OP_ADD %d,%d,%d\n", rd, rs1, rs2);
        this->GRegWrite(rd, this->GRegRead(rs1) + this->GRegRead(rs2));
        break;
      }

      case OP_SUB_func7: {
        printf("OP_SUB %d,%d,%d\n", rd, rs1, rs2);
        this->GRegWrite(rd, this->GRegRead(rs1) - this->GRegRead(rs2));
        break;
      }
      default: {
        return EXC_ILL_INSTR;
      }
      }
    }
    case OP_OR: {
      printf("OP_OR %d,%d,%d\n", rd, rs1, rs2);
      this->GRegWrite(rd, this->GRegRead(rs1) | this->GRegRead(rs2));
      break;
    }
    case OP_XOR: {
      printf("OP_XOR %d,%d,%d\n", rd, rs1, rs2);
      this->GRegWrite(rd, this->GRegRead(rs1) ^ this->GRegRead(rs2));
      break;
    }
    default: {
      return EXC_ILL_INSTR;
    }
    }
    this->IncrementPC(WORDLEN);
    break;
  }
  case I_TYPE: {
    printf("\tI-type | ");
    int8_t rs1 = RS1(instr);
    SWord imm = I_IMM(instr);
    uint8_t rd = RD(instr);
    switch (func3) {
    case OP_ADDI: {
      imm = SIGN_EXTENSION(imm, I_IMM_SIZE);
      printf("OP_ADDI %d,%d(%x),%d\n", rd, rs1, this->GRegRead(rs1), imm);
      this->GRegWrite(rd, this->GRegRead(rs1) + imm);
      break;
    }
    case OP_SLLI: {
      printf("OP_SLLI %d,%d,%d\n", rd, rs1, imm);
      this->GRegWrite(rd, this->GRegRead(rs1) << imm);
      break;
    }
    case OP_SLTI: {
      printf("OP_SLTI\n");
      imm = SIGN_EXTENSION(imm, I_IMM_SIZE);
      this->GRegWrite(rd, SWord(this->GRegRead(rs1)) < SWord(imm) ? 1 : 0);
      break;
    }
    case OP_SLTIU: {
      printf("OP_SLTIU\n");
      imm = SIGN_EXTENSION(imm, I_IMM_SIZE);
      this->GRegWrite(rd, this->GRegRead(rs1) < Word(imm) ? 1 : 0);
      break;
    }
    case OP_XORI: {
      printf("OP_XORI\n");
      imm = SIGN_EXTENSION(imm, I_IMM_SIZE);
      this->GRegWrite(rd, SWord(this->GRegRead(rs1)) ^ SWord(imm));
      break;
    }
    case OP_SR: {
      uint8_t func7 = FUNC7(instr);
      switch (func7) {
      case OP_SRLI_func7: {
        printf("OP_SRLI\n");
        this->GRegWrite(rd, this->GRegRead(rs1) >> imm);
        break;
      }
      case OP_SRAI_func7: {
        printf("OP_SRAI\n");
        uint8_t msb = rs1 & 0x80000000;
        this->GRegWrite(rd, this->GRegRead(rs1) >> imm | msb);
        break;
      }
      default:
        return EXC_ILL_INSTR;
      }
      this->GRegWrite(rd, SWord(this->GRegRead(rs1)) ^ SWord(imm));
      break;
    }
    case OP_ORI: {
      printf("OP_ORI\n");
      imm = SIGN_EXTENSION(imm, I_IMM_SIZE);
      this->GRegWrite(rd, SWord(this->GRegRead(rs1)) | SWord(imm));
      break;
    }
    case OP_ANDI: {
      printf("OP_ANDI\n");
      imm = SIGN_EXTENSION(imm, I_IMM_SIZE);
      this->GRegWrite(rd, SWord(this->GRegRead(rs1)) & SWord(imm));
      break;
    }
    default: {
      return EXC_ILL_INSTR;
    }
    }
    this->IncrementPC(WORDLEN);
    break;
  }
  case I2_TYPE: {
    printf("\tI2-type | ");
    uint16_t imm = I_IMM(instr);
    int8_t rs1 = RS1(instr);
    uint8_t rd = RD(instr);
    switch (func3) {
    case OP_ECALL_EBREAK: {
      switch (imm) {
      case ECALL_IMM: {
        // a7 - syscall number
        // a0 - result of syscall
        // https://www.cs.cornell.edu/courses/cs3410/2019sp/schedule/slides/14-ecf-pre.pdf
        printf("ECALL\n");
        switch (this->mode) {
        case MODE_USER: {
          return EXC_ENV_CALL_U;
        }
        case MODE_SUPERVISOR: {
          return EXC_ENV_CALL_S;
        }
        case MODE_MACHINE: {
          return EXC_ENV_CALL_M;
        }
        }
        break;
      }
      case EBREAK_IMM: {
        printf("EBREAK\n");
        return EXC_BREAKPOINT;
        break;
      }
      default: {
        return EXC_ILL_INSTR;
      }
      }
      break;
    }
    case OP_CSRRW: {
      std::cout << "OP_CSRRW\n";
      if (rd != 0x0)
        this->GRegWrite(rd, this->CSRRead(imm));
      if (rs1 != 0x0)
        this->CSRWrite(imm, this->GRegRead(rs1));
      this->IncrementPC(WORDLEN);
      break;
    }
    case OP_CSRRS: {
      std::cout << "OP_CSRRS\n";
      this->GRegWrite(rd, this->CSRRead(imm));
      this->CSRWrite(imm, this->CSRRead(imm) | this->GRegRead(rs1));
      this->IncrementPC(WORDLEN);
      break;
    }
    case OP_CSRRC: {
      std::cout << "OP_CSRRC\n";
      this->GRegWrite(rd, this->CSRRead(imm));
      if (rs1 != 0x0)
        this->CSRWrite(imm, this->CSRRead(imm) & ~this->GRegRead(rs1));
      this->IncrementPC(WORDLEN);
      break;
    }
    case OP_CSRRWI: {
      std::cout << "OP_CSRRWI\n";
      this->GRegWrite(rd, this->CSRRead(imm));
      this->CSRWrite(imm, this->CSRRead(imm) & ~rs1);
      this->IncrementPC(WORDLEN);
      break;
    }
    case OP_CSRRSI: {
      std::cout << "OP_CSRRSI\n";
      this->GRegWrite(rd, this->CSRRead(imm));
      this->CSRWrite(imm, this->CSRRead(imm) | rs1);
      this->IncrementPC(WORDLEN);
      break;
    }
    case OP_CSRRCI: {
      std::cout << "OP_CSRRCI\n";
      this->GRegWrite(rd, this->CSRRead(imm));
      this->CSRWrite(imm, this->CSRRead(imm) & ~rs1);
      this->IncrementPC(WORDLEN);
      break;
    }
    default: {
      return EXC_ILL_INSTR;
    }
    }
    break;
  }
  case B_TYPE: {
    printf("\tB-type | ");
    int8_t rs1 = RS1(instr);
    int8_t rs2 = RS2(instr);
    int16_t imm = B_IMM(instr);
    imm = SIGN_EXTENSION(imm, I_IMM_SIZE);
    switch (func3) {
    case OP_BEQ: {
      printf("OP_BEQ %d(%x),%d(%x),%x\n", rs1, this->GRegRead(rs1), rs2,
             this->GRegRead(rs2), imm);
      if (this->GRegRead(rs1) == this->GRegRead(rs2))
        this->SetPC((SWord)this->GetPC() + ((SWord)imm));
      else
        this->IncrementPC(WORDLEN);
      break;
    }
    case OP_BNE: {
      std::cout << "OP_BNE\n";
      if (this->GRegRead(rs1) != this->GRegRead(rs2))
        this->SetPC((SWord)this->GetPC() + ((SWord)imm));
      else
        this->IncrementPC(WORDLEN);
      break;
    }
    case OP_BLT: {
      std::cout << "OP_BLT\n";
      if (this->GRegRead(rs1) < this->GRegRead(rs2))
        this->SetPC((SWord)this->GetPC() + ((SWord)imm));
      else
        this->IncrementPC(WORDLEN);
      break;
    }
    case OP_BGE: {
      std::cout << "OP_BGE\n";
      if (this->GRegRead(rs1) >= this->GRegRead(rs2)) {
        this->SetPC((SWord)this->GetPC() + ((SWord)imm));
      } else
        this->IncrementPC(WORDLEN);
      break;
    }
    case OP_BLTU: {
      std::cout << "OP_BLTU\n";
      if (this->GRegRead(rs1) < this->GRegRead(rs2))
        this->SetPC(this->GetPC() + (imm));
      else
        this->IncrementPC(WORDLEN);
      break;
    }
    case OP_BGEU: {
      std::cout << "OP_BGEU\n";
      if (this->GRegRead(rs1) >= this->GRegRead(rs2))
        this->SetPC(this->GetPC() + (imm));
      else
        this->IncrementPC(WORDLEN);
      break;
    }
    default: {
      return EXC_ILL_INSTR;
    }
    }
    break;
  }
  case S_TYPE: {
    printf("\tS-type | ");
    int8_t rs1 = RS1(instr);
    int8_t rs2 = RS2(instr);
    int16_t imm = S_IMM(instr);
    imm = SIGN_EXTENSION(imm, S_IMM_SIZE);
    switch (func3) {
    case OP_SB: {
      printf("OP_SB\n");
      e = this->bus->Write((SWord)this->GRegRead(rs1) + (SWord)imm,
                           this->GRegRead(rs2) & 0xFF);
      this->IncrementPC(WORDLEN);
      break;
    }
    case OP_SH: {
      printf("OP_SH\n");
      e = this->bus->Write((SWord)this->GRegRead(rs1) + (SWord)imm,
                           this->GRegRead(rs2) & 0xFFFF);
      this->IncrementPC(WORDLEN);
      break;
    }
    case OP_SW: {
      printf("OP_SW\n");
      e = this->bus->Write((SWord)this->GRegRead(rs1) + (SWord)imm,
                           this->GRegRead(rs2));
      this->IncrementPC(WORDLEN);
      break;
    }
    default: {
      return EXC_ILL_INSTR;
    }
    }
    break;
  }
  case OP_AUIPC: {
    printf("\tU-type | OP_AUIPC\n");
    uint8_t rd = RD(instr);
    SWord imm = U_IMM(instr);
    imm = SIGN_EXTENSION(imm, I_IMM_SIZE);
    this->GRegWrite(rd, (SWord)this->GetPC() + imm);
    this->IncrementPC(WORDLEN);
    break;
  }
  case OP_LUI: {
    uint8_t rd = RD(instr);
    SWord imm = SIGN_EXTENSION(U_IMM(instr), U_IMM_SIZE) << 12;
    printf("\tU-type | OP_LUI %d,%x\n", rd, imm);
    this->GRegWrite(rd, imm);
    this->IncrementPC(WORDLEN);
    break;
  }
  case OP_JAL: {
    printf("\tJ-type | OP_JAL\n");
    uint8_t rd = RD(instr);
    Word imm = I_IMM(instr);
    imm = SIGN_EXTENSION(imm, I_IMM_SIZE) & 0xfffffffe;
    this->GRegWrite(rd, this->GetPC() + WORDLEN);
    this->IncrementPC(imm);
    break;
  }
  case OP_JALR: {
    printf("\tJ-type | OP_JALR\n");
    // TODO: understand diffs with JAL
    uint8_t rd = RD(instr);
    uint8_t rs1 = RS1(instr);
    Word imm = I_IMM(instr);
    imm = SIGN_EXTENSION(imm, I_IMM_SIZE);
    this->GRegWrite(rd, this->GetPC() + WORDLEN);
    this->IncrementPC((this->GRegRead(rs1) + imm) & 0xfffffffe);
    break;
  }
  case OP_FENCE: {
    printf("OP_FENCE\n!!! Da implementare\n");
    // printf("PR %d ", FENCE_PR(instr));
    // printf("PW %d ", FENCE_PW(instr));
    // printf("PI %d ", FENCE_PI(instr));
    // printf("PO %d ", FENCE_PO(instr));
    // printf("SR %d ", FENCE_SR(instr));
    // printf("SW %d ", FENCE_SW(instr));
    // printf("SI %d ", FENCE_SI(instr));
    // printf("SO %d\n", FENCE_SO(instr));
    this->IncrementPC(WORDLEN);
    break;
  }
  default: {
    return EXC_ILL_INSTR;
  }
  }
  printf("\n");

  return e;
}

exception_t MyProcessor::Cycle() {
  Word instr = 0;
  exception_t e = EXC_OK;
  e = this->Fetch(&instr);
  if (e != EXC_OK) {
    printf("Istruzione non trovata !!!\n");
    return e;
  }
  if ((e = this->Excecute(instr)) != EXC_OK) {
    if (e == EXC_ILL_INSTR) {
      printf("Istruzione non gestita !!!\n");
      return e;
    }
    TrapHandler(e);
  }
  if (DEBUG == 1) {
    Word tohost = 0;
    this->bus->Read(0x20001000, &tohost);
    if (tohost == 1) {
      printf("[-] Test passed !!!\n");
      return EXC_ILL_INSTR;
    }
  }

  // printf("SP %x | S0 %x | A5 %x\n", this->GRegRead(REG_SP),
  // this->GRegRead(REG_S0), this->GRegRead(REG_A0));

  return EXC_OK;
}

void MyProcessor::TrapHandler(exception_t e) {
  printf("Trap handler %d !!!\n", e);
  Word jumpto = 0x0;
  if (this->mode == MODE_MACHINE) {
    jumpto = this->CSRRead(MTVEC);
  } else if (this->mode == MODE_SUPERVISOR) {
    jumpto = this->CSRRead(STVEC);
  }
  // last two bits has to be 00
  jumpto &= ~0x3;

  Word sedeleg = this->CSRRead(SEDELEG);
  Word medeleg = this->CSRRead(MEDELEG);
  if (sedeleg) {
    printf("deleg to S\n");
    this->old_mode = this->mode;
    this->mode = MODE_MACHINE;
  } else if (medeleg) {
    printf("deleg to M\n");
    this->old_mode = this->mode;
    this->mode = MODE_MACHINE;
  }
  // TODO: check this one
  this->CSRWrite(MEPC, this->GetPC());
  this->CSRWrite(MCAUSE, e);
  this->CSRWrite(MTVAL, 0);
  // TODO: set MSTATUS
  printf("Jumping to %X\n", jumpto);
  this->SetPC(jumpto);
}

Word MyProcessor::GRegRead(Word reg) {
  return this->GetCurrentHart()->GRegRead(reg);
}
void MyProcessor::GRegWrite(Word reg, Word value) {
  this->GetCurrentHart()->GRegWrite(reg, value);
}
Word MyProcessor::CSRRead(Word reg) {
  return this->GetCurrentHart()->CSRRead(reg);
}
void MyProcessor::CSRWrite(Word reg, Word value) {

  this->GetCurrentHart()->CSRWrite(reg, value);
}

/*
 *
 * Legacy
 *
 */

void MyProcessor::Skip(uint32_t cycles) {
  // TODO
}

bool MyProcessor::InUserMode() const {
  // TODO
  return false;
}
bool MyProcessor::InKernelMode() const {
  // TODO
  return true;
}

uint32_t MyProcessor::IdleCycles() const {
  // TODO
  return 0;
}

void MyProcessor::Reset(Word pc, Word sp) {
  // TODO
  this->SetPC(pc);
  this->SetSP(sp);

  // maps PC to physical address space and fetches first instruction
  // mapVirtual and SystemBus cannot signal TRUE on this call
  if (mapVirtual(currPC, &currPhysPC, EXEC) ||
      bus->InstrRead(currPhysPC, &currInstr, this))
    Panic("Illegal memory access in MyProcessor::Reset");
}

void MyProcessor::Halt() {
  // TODO
}

void MyProcessor::SignalExc(unsigned int exc, Word cpuNum) {
  // TODO
}

void MyProcessor::AssertIRQ(unsigned int il) {
  // TODO
}
void MyProcessor::DeassertIRQ(unsigned int il) {
  // TODO
}
