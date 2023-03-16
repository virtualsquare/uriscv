#ifndef EXCHANDL_H_INCLUDED
#define EXCHANDL_H_INCLUDED

#include "globals.h"
#include "handlerFunction.h"
#include "uriscv/const.h"
#include "uriscv/cp0.h"
#include <uriscv/arch.h>
#include <uriscv/liburiscv.h>

/* Funzione di gestione delle eccezioni */
void exception_handler();

/*
 * La funzione chiama l'opportuno interrupt in base al primo device che trova in
 * funzione. Per vedere se un device è in funzione utilizziamo la macro 
 * AUSE_IP_GET che legge gli opportuni bit di CAUSE e restituisce 1 quando un
 * dispositivo è attivo. N.B.: La funzione CAUSE_GET_IP è ben commentata dov'è d
 * finita.
 */
void interrupt_handler(state_t *excState);

/* Funzioni per la gestione delle eccezioni trap, tlb e syscall */
void tlb_handler(state_t *callerProc);
void trap_handler(state_t *callerProc);
void syscall_handler(state_t *callerProc);

#endif
