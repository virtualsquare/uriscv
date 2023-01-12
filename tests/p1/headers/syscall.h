#ifndef SYSCALL_H_INCLUDED
#define SYSCALL_H_INCLUDED

#include "../generic_headers/pandos_const.h"
#include "./asl.h"
#include "./listx.h"
#include "./pcb.h"
#include "globals.h"
#include "handlerFunction.h"
#include "scheduler.h"
#include "uriscv/const.h"
#include "uriscv/cp0.h"
#include "uriscv/liburiscv.h"

extern void insert_ready_queue(int prio, pcb_PTR p);

/* Funzioni di aiuto per syscall */
void copy_state(state_t *new, state_t *old);
void term_single_proc(pcb_PTR p);
void term_proc_and_child(pcb_PTR rootPtr);
pcb_PTR find_pcb(int pid);
void block_curr_proc(state_t *excState, int *semaddr);
pcb_PTR free_process(int *semaddr);
void term_proc(int pid);
int lenQ(struct list_head *l);

void create_process(state_t *excState);
void terminate_process(state_t *excState);
void passeren(state_t *excState);
pcb_PTR P(int *semaddr, state_t *excState);
void verhogen(state_t *excState);
pcb_PTR V(int *semaddr, state_t *excState);
void do_IO_device(state_t *excState);
void get_cpu_time(state_t *excState);
void wait_for_clock(state_t *excState);
void get_support_data(state_t *excState);
void get_ID_process(state_t *excState);
void yield(state_t *excState);

#endif
