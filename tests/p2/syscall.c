#include "headers/syscall.h"

/* HELPER FUNCTION */

/**
 * It copies the contents of the new state into the old state
 *
 * @param new The new state that we want to copy
 * @param old The state that we want to copy to
 */
void copy_state(state_t *new, state_t *old) {
  old->cause = new->cause;
  old->entry_hi = new->entry_hi;
  for (int i = 0; i < STATE_GPR_LEN; i++)
    old->gpr[i] = new->gpr[i];
  old->hi = new->hi;
  old->lo = new->lo;
  old->pc_epc = new->pc_epc;
  old->status = new->status;
}

/**
 * It terminates all the children of a process, and then the process itself
 *
 * @param proc  the process to be terminated
 */
void term_proc_and_child(pcb_PTR proc) {
  pcb_PTR p;
  while (!isPcbFree(proc->p_pid)) {
    p = proc;
    while (!emptyChild(p))
      p = container_of(p->p_child.next, pcb_t, p_sib);

    term_single_proc(p); // termino p
  }
}

/**
 * It removes the process from the active process list, and if it's blocked, it
 * removes it from the blocked process list
 *
 * @param p the process to be terminated
 */
void term_single_proc(pcb_PTR p) {
  --activeProc;
  if (p->p_semAdd != NULL) {
    --blockedProc;
    outBlocked(p);
  }
  if (p == currentActiveProc)
    currentActiveProc = NULL;

  outChild(p); // take away p as son so it goes on
  freePcb(p);
}

/**
 * It searches through the list of active processes, through the list of blocked
 * processes then and returns the process with the given id.
 *
 * @param pid the process id of the process to be found
 *
 * @return The pcb_PTR of the process with the given pid.
 */
pcb_PTR find_pcb(int pid) {
  struct list_head *pos;
  pcb_PTR p;

  list_for_each(pos, &queueHighProc) {
    if ((p = container_of(pos, pcb_t, p_list))->p_pid == pid)
      return p;
  }

  list_for_each(pos, &queueLowProc) {
    if ((p = container_of(pos, pcb_t, p_list))->p_pid == pid)
      return p;
  }
  p = isPcbBlocked(pid);
  return p;
}

/**
 * It updates the current time process, saves the current process state, inserts
 * the process in the blocked queue, and calls the scheduler
 *
 * @param excState the state of the process that rised the exception
 * @param semaddr the address of the semaphore
 */
void block_curr_proc(state_t *excState, int *semaddr) {
  update_curr_proc_time();
  copy_state(excState, &currentActiveProc->p_s);
  insertBlocked(semaddr, currentActiveProc);
  ++blockedProc;
  scheduler();
}

/**
 * It removes a process from the blocked queue and inserts it into the ready
 * queue
 *
 * @param semaddr the address of the semaphore
 *
 * @return The process that was removed from the blocked queue.
 */
pcb_PTR free_process(int *semaddr) {
  pcb_PTR pid = removeBlocked(semaddr);
  --blockedProc;
  insert_ready_queue(pid->p_prio, pid);
  return pid;
}

/**
 * It terminates the process with the given pid and all of its children
 *
 * @param pid the process id of the process to be terminated.
 */
void term_proc(int pid) {
  pcb_PTR p;

  if (pid == 0)
    term_proc_and_child(currentActiveProc);
  else {
    p = find_pcb(pid);
    term_proc_and_child(p);
  }
}

/**
 * Calculate the length of a given list
 *
 * @param l the list_head pointer to the list you want to count
 *
 * @return The number of elements in the list.
 */
int lenQ(struct list_head *l) {
  int c = 0;
  struct list_head *tmp;
  list_for_each(tmp, l) { ++c; }
  return c;
}

/* SYSCALL */

// SYSCALL CREATEPROCESS
void create_process(state_t *excState) {
  pcb_PTR p = allocPcb();

  state_t *a1 = (state_t *)(*excState).reg_a1;
  int a2 = (int)(*excState).reg_a2;
  support_t *a3 = (support_t *)(*excState).reg_a3;

  if (p == NULL)
    (*excState).reg_a0 = NOPROC;
  else {
    insertChild(currentActiveProc, p);
    copy_state(a1, &p->p_s);
    ++activeProc;
    insert_ready_queue(a2, p);
    p->p_supportStruct = a3;
    if (a3 == 0)
      p->p_supportStruct = NULL;
    (*excState).reg_a0 = p->p_pid;
  }
  load_or_scheduler(excState);
}

// SYSCALL TERMPROCESS
void terminate_process(state_t *excState) {
  int pid = (int)(*excState).reg_a1;
  term_proc(pid);
  load_or_scheduler(excState);
}

// SYSCALL PASSEREN
void passeren(state_t *excState) {
  int *semaddr = (int *)(*excState).reg_a1;
  P(semaddr, excState);
  load_or_scheduler(excState);
}

/**
 * If the semaphore is available, decrement it and return NULL; otherwise, if
 * there are processes waiting on the semaphore, return the first one;
 * otherwise, block the current process and return NULL
 *
 * @param semaddr the address of the semaphore
 * @param excState the state of the process that is calling P
 *
 * @return The process that is being unblocked.
 */
pcb_PTR P(int *semaddr, state_t *excState) {
  pcb_PTR pid = headBlocked(semaddr);

  if ((*semaddr) == 0)
    block_curr_proc(excState, semaddr);
  else if (pid != NULL)
    return free_process(semaddr);
  else
    --(*semaddr);

  return NULL;
}

// SYSCALL VERHOGEN
void verhogen(state_t *excState) {
  int *semaddr = (int *)(*excState).reg_a1;
  V(semaddr, excState);
  load_or_scheduler(excState);
}

/**
 * If the semaphore is available, take it; otherwise, block the current process
 *
 * @param semaddr the address of the semaphore
 * @param excState the state of the process that is calling the V function
 *
 * @return The process that is being unblocked.
 */
pcb_PTR V(int *semaddr, state_t *excState) {
  pcb_PTR pid = headBlocked(semaddr);

  if ((*semaddr) == 1)
    block_curr_proc(excState, semaddr);
  else if (pid != NULL)
    return free_process(semaddr);
  else
    ++(*semaddr);
  return NULL;
}

// SYCALL DOIO
void do_IO_device(state_t *excState) {
  int *cmdAddr = (int *)(*excState).reg_a1;
  int cmdValue = (int)(*excState).reg_a2;

  int *devSemaphore = NULL; // Semaphore address
  devregarea_t *deviceRegs = (devregarea_t *)RAMBASEADDR;

  /* Searching which device is running looking first for terminal then for
   * generic devices */
  for (int i = 0; i < 8; i++) {
    if (&(deviceRegs->devreg[4][i].term.transm_command) ==
        (memaddr *)cmdAddr) { // Terminal Devices Writing
      devSemaphore = &semTerminalDeviceWriting[i];
      break;
    } else if (&(deviceRegs->devreg[4][i].term.recv_command) ==
               (memaddr *)cmdAddr) { // Terminal Devices Reading
      devSemaphore = &semTerminalDeviceReading[i];
      break;
    } else {
      for (int j = 0; j < 4; j++) {
        if ((memaddr *)&deviceRegs->devreg[j][i].dtp.command ==
            (memaddr *)cmdAddr) {
          if (j == 0)
            devSemaphore = &semDiskDevice[i];
          else if (j == 1)
            devSemaphore = &semFlashDevice[i];
          else if (j == 2)
            devSemaphore = &semNetworkDevice[i];
          else
            devSemaphore = &semPrinterDevice[i];
          break;
        }
      }
    }
  }

  *cmdAddr = cmdValue; // Execute request command
  P(devSemaphore,
    excState); // Call a P on the semaphore found, should be blocking
  load_or_scheduler(excState); // Just in case
}

// SYSCALL GETTIME
void get_cpu_time(state_t *excState) {
  update_curr_proc_time();
  excState->reg_a0 = currentActiveProc->p_time;
  load_or_scheduler(excState);
}

// SYSCALL CLOCKWAIT
void wait_for_clock(state_t *excState) {
  P(&semIntervalTimer, excState); // should be blocking
  load_or_scheduler(excState);    // Just in case
}

// SYSCALL GETSUPPORTPTR
void get_support_data(state_t *excState) {
  (*excState).reg_a0 =
      (unsigned int)((currentActiveProc->p_supportStruct != NULL)
                         ? currentActiveProc->p_supportStruct
                         : NULL);
  load_or_scheduler(excState);
}

// SYSCALL GETPROCESSID
void get_ID_process(state_t *excState) {
  int parent = (int)(*excState).reg_a1;
  if (parent == 0)
    (*excState).reg_a0 = currentActiveProc->p_pid;
  else
    (*excState).reg_a0 = currentActiveProc->p_parent->p_pid;

  load_or_scheduler(excState);
}

// SYSCALL YIELD
void yield(state_t *excState) {
  copy_state(excState, &currentActiveProc->p_s);
  if (currentActiveProc->p_prio == PROCESS_PRIO_HIGH)
    yieldHighProc = currentActiveProc;
  else
    insert_ready_queue(currentActiveProc->p_prio, currentActiveProc);

  scheduler();
}
