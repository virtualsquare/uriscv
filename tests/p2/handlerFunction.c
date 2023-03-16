#include "headers/handlerFunction.h"

/* INTERRUPT HANDLER FUNCTION */

// handler IL_CPUTIMER
void plt_time_handler(state_t *excState) {
  setTIMER(-2); // ACK
  if (currentActiveProc != 0x0) {
    copy_state(excState, &currentActiveProc->p_s);
    insert_ready_queue(currentActiveProc->p_prio, currentActiveProc);
  }
  scheduler();
}

// handler IL_TIMER
void intervall_timer_handler(state_t *excState) {
  LDIT(100000); // ACK
  pcb_PTR p;
  while ((p = removeBlocked(&semIntervalTimer)) != NULL) {
    --blockedProc;
    insert_ready_queue(p->p_prio, p);
  }
  semIntervalTimer = 0;
  load_or_scheduler(excState);
}

/**
 * Obtain the semaphore address of a generic device (NON TERMINAL)
 * Take the device's interrupt line and the device number as input (from 0 to 7)
 * and return semaphore's address
 *
 * @param interruptLine The interrupt line yo recognize.
 * @param devNumber The device index of the semaphore array
 *
 * @return A pointer to the semaphore for the device.
 */
int *getDeviceSemaphore(int interruptLine, int devNumber) {
  switch (interruptLine) {
  case IL_DISK:
    return &semDiskDevice[devNumber];
  case IL_FLASH:
    return &semFlashDevice[devNumber];
  case IL_ETHERNET:
    return &semNetworkDevice[devNumber];
  case IL_PRINTER:
    return &semPrinterDevice[devNumber];
  }
  return NULL;
}

// handler IL_DISK | IL_FLASH | IL_ETHERNET | IL_PRINTER | IL_TERMINAL
void device_handler(int interLine, state_t *excState) {
  unsigned int statusCode;
  int *devSemaphore = NULL;
  int devNumber = 0;

  devregarea_t *devRegs = (devregarea_t *)RAMBASEADDR;
  unsigned int bitmap = devRegs->interrupt_dev[interLine - 3];
  unsigned int mask = 1;

  for (int i = 0; i < N_DEV_PER_IL; i++) {
    if (bitmap & mask) {
      devNumber = i;

      if (interLine == IL_TERMINAL) {
        termreg_t *devRegAddr = (termreg_t *)DEV_REG_ADDR(interLine, devNumber);

        if (devRegAddr->transm_status != READY &&
            devRegAddr->transm_status != BUSY) {
          statusCode = devRegAddr->transm_status;
          devRegAddr->transm_command = ACK;
          devSemaphore = &semTerminalDeviceWriting[devNumber];
        } else {
          statusCode = devRegAddr->recv_status;
          devRegAddr->recv_command = ACK;
          devSemaphore = &semTerminalDeviceReading[devNumber];
        }
      } else {
        dtpreg_t *devRegAddr = (dtpreg_t *)DEV_REG_ADDR(interLine, devNumber);
        devSemaphore = getDeviceSemaphore(interLine, devNumber);
        statusCode = devRegAddr->status; // Save status code
        devRegAddr->command = ACK;       // Acknowledge the interrupt
      }
    }
    mask *= 2;
  }

  /* V-Operation */
  pcb_PTR proc = V(devSemaphore, NULL);

  if (proc == NULL || proc == currentActiveProc) {
    currentActiveProc->p_s.reg_a0 = statusCode;
    insert_ready_queue(currentActiveProc->p_prio, currentActiveProc);
    scheduler();
  } else {
    proc->p_s.reg_a0 = statusCode;
    load_or_scheduler(excState);
  }
}

// Handler TLB | TRAP
void pass_up_or_die(int pageFault, state_t *excState) {
  if (currentActiveProc != NULL) {
    if (currentActiveProc->p_supportStruct == NULL) {
      term_proc(0);
      scheduler();
    } else {
      copy_state(
          excState,
          &currentActiveProc->p_supportStruct->sup_exceptState[pageFault]);
      int stackPtr =
          currentActiveProc->p_supportStruct->sup_exceptContext[pageFault]
              .stackPtr;
      int status =
          currentActiveProc->p_supportStruct->sup_exceptContext[pageFault]
              .status;
      int pc =
          currentActiveProc->p_supportStruct->sup_exceptContext[pageFault].pc;
      LDCXT(stackPtr, status, pc);
    }
  }
}
