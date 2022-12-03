#include "uriscv/hart.h"
#include "uriscv/mybus.h"
#include <cstdio>

Hart::Hart(MyBus *bus) { this->bus = bus; }

void Hart::Init(Word id, Word pc, Word sp) {
  this->id = id;
  this->pc = pc;
  this->sp = sp;
  for (uint8_t i = 0; i < NUM_REGS; i++) {
    this->gregs[i] = 0;
  }
}
//
// physical pc, not virtual
exception_t Hart::Fetch(Word pc, Word *instr) {
  exception_t e = this->bus->Read(pc, instr);
  if (e != EXC_OK)
    // TODO: signal error
    return EXC_INSTR_ACC_FAULT;

  return EXC_OK;
}

exception_t Hart::Fetch(Word *instr) { return this->Fetch(this->pc, instr); }

Word Hart::GRegRead(Word reg) { return gregs[reg]; }
void Hart::GRegWrite(Word reg, Word value) {
  // reg 0 in hardwired to 0
  if (reg != 0)
    gregs[reg] = value;
}
Word Hart::CSRRead(Word reg) {
  // TODO : check perm
  return csrs[reg].value;
}
void Hart::CSRWrite(Word reg, Word value) {
  // TODO : check perm
  csrs[reg].value = value;
}

void Hart::SetPC(Word value) { this->pc = value; }
void Hart::SetSP(Word value) { this->sp = value; }
Word Hart::IncrementPC(Word value) {
  this->pc = (SWord)this->pc + (SWord)value;
  return this->pc;
}
Word Hart::IncrementSP(Word value) {
  this->sp += value;
  return this->sp;
}

Word Hart::GetPC() { return this->pc; }

Word Hart::GetSP() { return this->sp; }

void Hart::initCSR() {
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

  this->csrs[UTVEC].perm = WWW;
  this->csrs[STVEC].perm = NWW;
  this->csrs[MTVEC].perm = NNW;

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
