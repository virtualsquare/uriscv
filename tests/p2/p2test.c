/* File: $Id: p2test.c,v 1.1 1998/01/20 09:28:08 morsiani Exp morsiani $ */

/*********************************P2TEST.C*******************************
 *
 *	Test program for the PandosPlus Kernel: phase 2.
 *      v.0.1: March 20, 2022
 *
 *	Produces progress messages on Terminal0.
 *
 *	This is pretty convoluted code, so good luck!
 *
 *		Aborts as soon as an error is detected.
 *
 *      Modified by Michael Goldweber on May 15, 2004
 *		Modified by Michael Goldweber on June 19, 2020
 */

#include "./generic_headers/pandos_const.h"
#include "./generic_headers/pandos_types.h"
#include <uriscv/liburiscv.h>

typedef unsigned int devregtr;

/* hardware constants */
#define PRINTCHR 2
#define RECVD 5

#define CLOCKINTERVAL 100000UL /* interval to V clock semaphore */

#define TERMSTATMASK 0xFF
#define CAUSEMASK 0xFF
#define VMOFF 0xF8FFFFFF

#define SYSCAUSE (0x8 << 2)
#define BUSERROR 6
#define RESVINSTR 10
#define ADDRERROR 4
#define SYSCALLEXCPT 8

#define QPAGE 1024

#define IEPBITON 0x4
#define KUPBITON 0x8
#define KUPBITOFF 0xFFFFFFF7
#define TEBITON 0x08000000

#define CAUSEINTMASK 0xFD00
#define CAUSEINTOFFS 10

#define MINLOOPTIME 30000
#define LOOPNUM 10000

#define CLOCKLOOP 10
#define MINCLOCKLOOP 3000

#define BADADDR 0xFFFFFFFF
#define TERM0ADDR 0x10000254

/* just to be clear */
#define NOLEAVES 4 /* number of leaves of p8 process tree */
#define MAXSEM 20

int sem_term_mut = 1, /* for mutual exclusion on terminal */
    s[MAXSEM + 1],    /* semaphore array */
    sem_testsem = 0,  /* for a simple test */
    sem_startp2 = 0,  /* used to start p2 */
    sem_endp2 =
        1, /* used to signal p2's demise (test binary blocking V on binary sem*/
    sem_endp3 = 0, /* used to signal p3's demise */
    sem_blkp4 = 1, /* used to block second incaration of p4 */
    sem_synp4 = 0, /* used to allow p4 incarnations to synhronize */
    sem_endp4 = 0, /* to signal demise of p4 */
    sem_endp5 = 0, /* to signal demise of p5 */
    sem_endp8 = 0, /* to signal demise of p8 */
    sem_endcreate[NOLEAVES] = {0}, /* for a p8 leaf to signal its creation */
    sem_blkp8 = 0,                 /* to block p8 */
    sem_blkp9 = 0;                 /* to block p9 */

state_t p2state, p3state, p4state, p5state, p6state, p7state, p8rootstate,
    child1state, child2state, gchild1state, gchild2state, gchild3state,
    gchild4state, p9state, p10state, hp_p1state, hp_p2state;

int p2pid, p3pid, p4pid, p8pid, p9pid;

/* support structure for p5 */
support_t pFiveSupport;

int p1p2synch = 0; /* to check on p1/p2 synchronization */

int p8inc;     /* p8's incarnation number */
int p4inc = 1; /* p4 incarnation number */

unsigned int p5Stack; /* so we can allocate new stack for 2nd p5 */

int creation = 0;           /* return code for SYSCALL invocation */
memaddr *p5MemLocation = 0; /* To cause a p5 trap */

void p2(), p3(), p4(), p5(), p5a(), p5b(), p6(), p7(), p7a(), p5prog(), p5mm();
void p5sys(), p8root(), child1(), child2(), p8leaf1(), p8leaf2(), p8leaf3(),
    p8leaf4(), p9(), p10(), hp_p1(), hp_p2();

extern void p5gen();
extern void p5mm();

/* a procedure to print on terminal 0 */
void print(char *msg) {

  char *s = msg;
  devregtr *base = (devregtr *)(TERM0ADDR);
  devregtr *command = base + 3;
  devregtr status;

  SYSCALL(PASSEREN, (int)&sem_term_mut, 0, 0); /* P(sem_term_mut) */
  while (*s != EOS) {
    devregtr value = PRINTCHR | (((devregtr)*s) << 8);
    status = SYSCALL(DOIO, (int)command, (int)value, 0);
    if ((status & TERMSTATMASK) != RECVD) {
      PANIC();
    }
    s++;
  }
  SYSCALL(VERHOGEN, (int)&sem_term_mut, 0, 0); /* V(sem_term_mut) */
}

/* TLB-Refill Handler */
/* One can place debug calls here, but not calls to print */
void uTLB_RefillHandler() {

  // TODO: riattivare
  // setENTRYHI(0x80000000);
  // setENTRYLO(0x00000000);
  // TLBWR();

  LDST((state_t *)0x0FFFF000);
}

/*********************************************************************/
/*                                                                   */
/*                 p1 -- the root process                            */
/*                                                                   */
void test() {
  SYSCALL(VERHOGEN, (int)&sem_testsem, 0, 0); /* V(sem_testsem)   */

  print("p1 v(sem_testsem)\n");

  /* set up states of the other processes */

  STST(&hp_p1state);
  hp_p1state.reg_sp = hp_p1state.reg_sp - QPAGE;
  hp_p1state.pc_epc = (memaddr)hp_p1;
  hp_p1state.status = hp_p1state.status | IEPBITON | CAUSEINTMASK | TEBITON;

  STST(&hp_p2state);
  hp_p2state.reg_sp = hp_p1state.reg_sp - QPAGE;
  hp_p2state.pc_epc = (memaddr)hp_p2;
  hp_p2state.status = hp_p2state.status | IEPBITON | CAUSEINTMASK | TEBITON;

  STST(&p2state);
  p2state.reg_sp = hp_p2state.reg_sp - QPAGE;
  p2state.pc_epc = (memaddr)p2;
  p2state.status = p2state.status | IEPBITON | CAUSEINTMASK | TEBITON;

  STST(&p3state);
  p3state.reg_sp = p2state.reg_sp - QPAGE;
  p3state.pc_epc = (memaddr)p3;
  p3state.status = p3state.status | IEPBITON | CAUSEINTMASK | TEBITON;

  STST(&p4state);
  p4state.reg_sp = p3state.reg_sp - QPAGE;
  p4state.pc_epc = (memaddr)p4;
  p4state.status = p4state.status | IEPBITON | CAUSEINTMASK | TEBITON;

  STST(&p5state);
  p5Stack = p5state.reg_sp =
      p4state.reg_sp - (2 * QPAGE); /* because there will 2 p4 running*/
  p5state.pc_epc = (memaddr)p5;
  p5state.status = p5state.status | IEPBITON | CAUSEINTMASK | TEBITON;

  STST(&p6state);
  p6state.reg_sp = p5state.reg_sp - (2 * QPAGE);
  p6state.pc_epc = (memaddr)p6;
  p6state.status = p6state.status | IEPBITON | CAUSEINTMASK | TEBITON;

  STST(&p7state);
  p7state.reg_sp = p6state.reg_sp - QPAGE;
  p7state.pc_epc = (memaddr)p7;
  p7state.status = p7state.status | IEPBITON | CAUSEINTMASK | TEBITON;

  STST(&p8rootstate);
  p8rootstate.reg_sp = p7state.reg_sp - QPAGE;
  p8rootstate.pc_epc = (memaddr)p8root;
  p8rootstate.status = p8rootstate.status | IEPBITON | CAUSEINTMASK | TEBITON;

  STST(&child1state);
  child1state.reg_sp = p8rootstate.reg_sp - QPAGE;
  child1state.pc_epc = (memaddr)child1;
  child1state.status = child1state.status | IEPBITON | CAUSEINTMASK | TEBITON;

  STST(&child2state);
  child2state.reg_sp = child1state.reg_sp - QPAGE;
  child2state.pc_epc = (memaddr)child2;
  child2state.status = child2state.status | IEPBITON | CAUSEINTMASK | TEBITON;

  STST(&gchild1state);
  gchild1state.reg_sp = child2state.reg_sp - QPAGE;
  gchild1state.pc_epc = (memaddr)p8leaf1;
  gchild1state.status = gchild1state.status | IEPBITON | CAUSEINTMASK | TEBITON;

  STST(&gchild2state);
  gchild2state.reg_sp = gchild1state.reg_sp - QPAGE;
  gchild2state.pc_epc = (memaddr)p8leaf2;
  gchild2state.status = gchild2state.status | IEPBITON | CAUSEINTMASK | TEBITON;

  STST(&gchild3state);
  gchild3state.reg_sp = gchild2state.reg_sp - QPAGE;
  gchild3state.pc_epc = (memaddr)p8leaf3;
  gchild3state.status = gchild3state.status | IEPBITON | CAUSEINTMASK | TEBITON;

  STST(&gchild4state);
  gchild4state.reg_sp = gchild3state.reg_sp - QPAGE;
  gchild4state.pc_epc = (memaddr)p8leaf4;
  gchild4state.status = gchild4state.status | IEPBITON | CAUSEINTMASK | TEBITON;

  STST(&p9state);
  p9state.reg_sp = gchild4state.reg_sp - QPAGE;
  p9state.pc_epc = (memaddr)p9;
  p9state.status = p9state.status | IEPBITON | CAUSEINTMASK | TEBITON;

  STST(&p10state);
  p10state.reg_sp = p9state.reg_sp - QPAGE;
  p10state.pc_epc = (memaddr)p10;
  p10state.status = p10state.status | IEPBITON | CAUSEINTMASK | TEBITON;

  /* create process p2 */
  p2pid = SYSCALL(CREATEPROCESS, (int)&p2state, PROCESS_PRIO_LOW,
                  (int)NULL); /* start p2     */

  print("p2 was started\n");

  SYSCALL(VERHOGEN, (int)&sem_startp2, 0, 0); /* V(sem_startp2)   */

  SYSCALL(VERHOGEN, (int)&sem_endp2, 0, 0); /* V(sem_endp2) (blocking V!)     */

  /* make sure we really blocked */
  if (p1p2synch == 0) {
    print("error: p1/p2 synchronization bad\n");
  }

  p3pid = SYSCALL(CREATEPROCESS, (int)&p3state, PROCESS_PRIO_LOW,
                  (int)NULL); /* start p3     */

  print("p3 is started\n");

  SYSCALL(PASSEREN, (int)&sem_endp3, 0, 0); /* P(sem_endp3)     */

  SYSCALL(CREATEPROCESS, (int)&hp_p1state, PROCESS_PRIO_HIGH, (int)NULL);
  SYSCALL(CREATEPROCESS, (int)&hp_p2state, PROCESS_PRIO_HIGH, (int)NULL);

  p4pid = SYSCALL(CREATEPROCESS, (int)&p4state, PROCESS_PRIO_LOW,
                  (int)NULL); /* start p4     */

  pFiveSupport.sup_exceptContext[GENERALEXCEPT].stackPtr = (int)p5Stack;
  pFiveSupport.sup_exceptContext[GENERALEXCEPT].status =
      ALLOFF | IEPBITON | CAUSEINTMASK | TEBITON;
  pFiveSupport.sup_exceptContext[GENERALEXCEPT].pc = (memaddr)p5gen;
  pFiveSupport.sup_exceptContext[PGFAULTEXCEPT].stackPtr = p5Stack;
  pFiveSupport.sup_exceptContext[PGFAULTEXCEPT].status =
      ALLOFF | IEPBITON | CAUSEINTMASK | TEBITON;
  pFiveSupport.sup_exceptContext[PGFAULTEXCEPT].pc = (memaddr)p5mm;

  SYSCALL(CREATEPROCESS, (int)&p5state, PROCESS_PRIO_LOW,
          (int)&(pFiveSupport)); /* start p5     */

  SYSCALL(CREATEPROCESS, (int)&p6state, PROCESS_PRIO_LOW,
          (int)NULL); /* start p6		*/

  SYSCALL(CREATEPROCESS, (int)&p7state, PROCESS_PRIO_LOW,
          (int)NULL); /* start p7		*/

  p9pid = SYSCALL(CREATEPROCESS, (int)&p9state, PROCESS_PRIO_LOW,
                  (int)NULL); /* start p7		*/

  SYSCALL(PASSEREN, (int)&sem_endp5, 0, 0); /* P(sem_endp5)		*/

  print("p1 knows p5 ended\n");

  SYSCALL(PASSEREN, (int)&sem_blkp4, 0, 0); /* P(sem_blkp4)		*/

  /* now for a more rigorous check of process termination */
  for (p8inc = 0; p8inc < 4; p8inc++) {
    /* Reset semaphores */
    sem_blkp8 = 0;
    sem_endp8 = 0;
    for (int i = 0; i < NOLEAVES; i++) {
      sem_endcreate[i] = 0;
    }

    p8pid =
        SYSCALL(CREATEPROCESS, (int)&p8rootstate, PROCESS_PRIO_LOW, (int)NULL);

    SYSCALL(PASSEREN, (int)&sem_endp8, 0, 0);
  }

  print("p1 finishes OK -- TTFN\n");
  *((memaddr *)BADADDR) = 0; /* terminate p1 */

  /* should not reach this point, since p1 just got a program trap */
  print("error: p1 still alive after progtrap & no trap vector\n");
  PANIC(); /* PANIC !!!     */
}

/* p2 -- semaphore and cputime-SYS test process */
void p2() {
  int i;                /* just to waste time  */
  cpu_t now1, now2;     /* times of day        */
  cpu_t cpu_t1, cpu_t2; /* cpu time used       */

  SYSCALL(PASSEREN, (int)&sem_startp2, 0, 0); /* P(sem_startp2)   */

  print("p2 starts\n");

  int pid = SYSCALL(GETPROCESSID, 0, 0, 0);
  if (pid != p2pid) {
    print("Inconsistent process id for p2!\n");
    PANIC();
  }

  /* initialize all semaphores in the s[] array */
  for (i = 0; i <= MAXSEM; i++) {
    s[i] = 0;
  }

  /* V, then P, all of the semaphores in the s[] array */
  for (i = 0; i <= MAXSEM; i++) {
    SYSCALL(VERHOGEN, (int)&s[i], 0, 0); /* V(S[I]) */
    SYSCALL(PASSEREN, (int)&s[i], 0, 0); /* P(S[I]) */
    if (s[i] != 0)
      print("error: p2 bad v/p pairs\n");
  }

  print("p2 v's successfully\n");

  /* test of SYS6 */

  STCK(now1);                         /* time of day   */
  cpu_t1 = SYSCALL(GETTIME, 0, 0, 0); /* CPU time used */

  /* delay for several milliseconds */
  for (i = 1; i < LOOPNUM; i++)
    ;

  cpu_t2 = SYSCALL(GETTIME, 0, 0, 0); /* CPU time used */
  STCK(now2);                         /* time of day  */

  // if (((now2 - now1) >= (cpu_t2 - cpu_t1)) &&
  //     ((cpu_t2 - cpu_t1) >= (MINLOOPTIME / (*((cpu_t *)TIMESCALEADDR))))) {
  //   print("p2 is OK\n");
  // } else {
  //   if ((now2 - now1) < (cpu_t2 - cpu_t1))
  //     print("error: more cpu time than real time\n");
  //   if ((cpu_t2 - cpu_t1) < (MINLOOPTIME / (*((cpu_t *)TIMESCALEADDR))))
  //     print("error: not enough cpu time went by\n");
  //   print("p2 blew it!\n");
  // }

  p1p2synch = 1; /* p1 will check this */

  SYSCALL(PASSEREN, (int)&sem_endp2, 0, 0); /* P(sem_endp2)    unblocking P ! */

  SYSCALL(TERMPROCESS, 0, 0, 0); /* terminate p2 */

  /* just did a SYS2, so should not get to this point */
  print("error: p2 didn't terminate\n");
  PANIC(); /* PANIC!           */
}

/* p3 -- clock semaphore test process */
void p3() {
  cpu_t time1, time2;
  cpu_t cpu_t1, cpu_t2; /* cpu time used       */
  int i;

  time1 = 0;
  time2 = 0;

  /* loop until we are delayed at least half of clock V interval */
  while (time2 - time1 < (CLOCKINTERVAL >> 1)) {
    STCK(time1); /* time of day     */
    SYSCALL(CLOCKWAIT, 0, 0, 0);
    STCK(time2); /* new time of day */
  }

  print("p3 - CLOCKWAIT OK\n");

  /* now let's check to see if we're really charge for CPU
     time correctly */
  cpu_t1 = SYSCALL(GETTIME, 0, 0, 0);

  for (i = 0; i < CLOCKLOOP; i++) {
    SYSCALL(CLOCKWAIT, 0, 0, 0);
  }

  cpu_t2 = SYSCALL(GETTIME, 0, 0, 0);

  // if (cpu_t2 - cpu_t1 < (MINCLOCKLOOP / (*((cpu_t *)TIMESCALEADDR)))) {
  //   print("error: p3 - CPU time incorrectly maintained\n");
  // } else {
  //   print("p3 - CPU time correctly maintained\n");
  // }

  int pid = SYSCALL(GETPROCESSID, 0, 0, 0);
  if (pid != p3pid) {
    print("Inconsistent process id for p3!\n");
    PANIC();
  }

  SYSCALL(VERHOGEN, (int)&sem_endp3, 0, 0); /* V(sem_endp3)        */

  SYSCALL(TERMPROCESS, 0, 0, 0); /* terminate p3    */

  /* just did a SYS2, so should not get to this point */
  print("error: p3 didn't terminate\n");
  PANIC(); /* PANIC            */
}

/* p4 -- termination test process */
void p4() {
  switch (p4inc) {
  case 1:
    print("first incarnation of p4 starts\n");
    p4inc++;
    break;

  case 2:
    print("second incarnation of p4 starts\n");
    break;
  }

  int pid = SYSCALL(GETPROCESSID, 0, 0, 0);
  if (pid != p4pid) {
    print("Inconsistent process id for p4!\n");
    PANIC();
  }

  SYSCALL(VERHOGEN, (int)&sem_synp4, 0, 0); /* V(sem_synp4)     */

  SYSCALL(PASSEREN, (int)&sem_blkp4, 0, 0); /* P(sem_blkp4)     */

  SYSCALL(PASSEREN, (int)&sem_synp4, 0, 0); /* P(sem_synp4)     */

  /* start another incarnation of p4 running, and wait for  */
  /* a V(sem_synp4). the new process will block at the P(sem_blkp4),*/
  /* and eventually, the parent p4 will terminate, killing  */
  /* off both p4's.                                         */

  p4state.reg_sp -= QPAGE; /* give another page  */

  p4pid = SYSCALL(CREATEPROCESS, (int)&p4state, PROCESS_PRIO_LOW,
                  0); /* start a new p4    */

  SYSCALL(PASSEREN, (int)&sem_synp4, 0, 0); /* wait for it       */

  print("p4 is OK\n");

  SYSCALL(VERHOGEN, (int)&sem_endp4, 0, 0); /* V(sem_endp4)          */

  SYSCALL(TERMPROCESS, 0, 0, 0); /* terminate p4      */

  /* just did a SYS2, so should not get to this point */
  print("error: p4 didn't terminate\n");
  PANIC(); /* PANIC            */
}

/* p5's program trap handler */
void p5gen() {
  unsigned int exeCode = pFiveSupport.sup_exceptState[GENERALEXCEPT].cause;
  exeCode = (exeCode & CAUSEMASK) >> 2;
  switch (exeCode) {
  case BUSERROR:
    print("Bus Error (as expected): Access non-existent memory\n");
    pFiveSupport.sup_exceptState[GENERALEXCEPT].pc_epc =
        (memaddr)p5a; /* Continue with p5a() */
    break;

  case RESVINSTR:
    print("privileged instruction\n");
    /* return in kernel mode */
    pFiveSupport.sup_exceptState[GENERALEXCEPT].pc_epc =
        (memaddr)p5b; /* Continue with p5b() */
    pFiveSupport.sup_exceptState[GENERALEXCEPT].status =
        pFiveSupport.sup_exceptState[GENERALEXCEPT].status & KUPBITOFF;
    break;

  case ADDRERROR:
    print("Address Error (as expected): non-kuseg access w/KU=1\n");
    /* return in kernel mode */
    pFiveSupport.sup_exceptState[GENERALEXCEPT].pc_epc =
        (memaddr)p5b; /* Continue with p5b() */
    pFiveSupport.sup_exceptState[GENERALEXCEPT].status =
        pFiveSupport.sup_exceptState[GENERALEXCEPT].status & KUPBITOFF;
    break;

  case SYSCALLEXCPT:
    p5sys();
    break;

  default:
    print("other program trap\n");
  }

  LDST(&(pFiveSupport.sup_exceptState[GENERALEXCEPT]));
}

/* p5's memory management trap handler */
void p5mm() {
  print("memory management trap\n");

  support_t *pFiveSupAddr = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
  if ((pFiveSupAddr) != &(pFiveSupport)) {
    print("Support Structure Address Error\n");
  } else {
    print("Correct Support Structure Address\n");
  }

  pFiveSupport.sup_exceptState[PGFAULTEXCEPT].status =
      pFiveSupport.sup_exceptState[PGFAULTEXCEPT].status |
      KUPBITON; /* user mode on 	*/
  pFiveSupport.sup_exceptState[PGFAULTEXCEPT].pc_epc =
      (memaddr)p5b; /* return to p5b()	*/

  LDST(&(pFiveSupport.sup_exceptState[PGFAULTEXCEPT]));
}

/* p5's SYS trap handler */
void p5sys() {
  unsigned int p5status = pFiveSupport.sup_exceptState[GENERALEXCEPT].status;
  p5status = (p5status << 28) >> 31;
  switch (p5status) {
  case ON:
    print("High level SYS call from user mode process\n");
    break;

  case OFF:
    print("High level SYS call from kernel mode process\n");
    break;
  }
  pFiveSupport.sup_exceptState[GENERALEXCEPT].pc_epc =
      pFiveSupport.sup_exceptState[GENERALEXCEPT].pc_epc +
      4; /*	 to avoid SYS looping */
  LDST(&(pFiveSupport.sup_exceptState[GENERALEXCEPT]));
}

/* p5 -- SYS5 test process */
void p5() {
  print("p5 starts\n");

  /* cause a pgm trap access some non-existent memory */
  *p5MemLocation = *p5MemLocation + 1; /* Should cause a program trap */
}

void p5a() {
  /* generage a TLB exception after a TLB-Refill event */

  p5MemLocation = (memaddr *)0x80000000;
  *p5MemLocation = 42;
}

/* second part of p5 - should be entered in user mode first time through */
/* should generate a program trap (Address error) */
void p5b() {
  cpu_t time1, time2;

  SYSCALL(1, 0, 0, 0);
  SYSCALL(PASSEREN, (int)&sem_endp4, 0, 0); /* P(sem_endp4)*/

  /* do some delay to be reasonably sure p4 and its offspring are dead */
  time1 = 0;
  time2 = 0;
  while (time2 - time1 < (CLOCKINTERVAL >> 1)) {
    STCK(time1);
    SYSCALL(CLOCKWAIT, 0, 0, 0);
    STCK(time2);
  }

  /* if p4 and offspring are really dead, this will increment sem_blkp4 */

  SYSCALL(VERHOGEN, (int)&sem_blkp4, 0, 0); /* V(sem_blkp4) */
  SYSCALL(VERHOGEN, (int)&sem_endp5, 0, 0); /* V(sem_endp5) */

  /* should cause a termination       */
  /* since this has already been      */
  /* done for PROGTRAPs               */

  SYSCALL(TERMPROCESS, 0, 0, 0);

  /* should have terminated, so should not get to this point */
  print("error: p5 didn't terminate\n");
  PANIC(); /* PANIC            */
}

/*p6 -- high level syscall without initializing passup vector */
void p6() {
  print("p6 starts\n");

  SYSCALL(1, 0, 0, 0); /* should cause termination because p6 has no
         trap vector */

  print("error: p6 alive after SYS9() with no trap vector\n");

  PANIC();
}

/*p7 -- program trap without initializing passup vector */
void p7() {
  print("p7 starts\n");

  *((memaddr *)BADADDR) = 0;

  print("error: p7 alive after program trap with no trap vector\n");
  PANIC();
}

/* p8root -- test of termination of subtree of processes              */
/* create a subtree of processes, wait for the leaves to block, signal*/
/* the root process, and then terminate                               */
void p8root() {
  int grandchild;

  print("p8root starts\n");

  SYSCALL(CREATEPROCESS, (int)&child1state, PROCESS_PRIO_LOW, (int)NULL);

  SYSCALL(CREATEPROCESS, (int)&child2state, PROCESS_PRIO_LOW, (int)NULL);

  for (grandchild = 0; grandchild < NOLEAVES; grandchild++) {
    SYSCALL(PASSEREN, (int)&sem_endcreate[grandchild], 0, 0);
  }

  SYSCALL(VERHOGEN, (int)&sem_endp8, 0, 0);

  SYSCALL(TERMPROCESS, 0, 0, 0);
}

/*child1 & child2 -- create two sub-processes each*/

void child1() {
  print("child1 starts\n");

  int ppid = SYSCALL(GETPROCESSID, 1, 0, 0);
  if (ppid != p8pid) {
    print("Inconsistent (parent) process id for p8's first child\n");
    PANIC();
  }

  SYSCALL(CREATEPROCESS, (int)&gchild1state, PROCESS_PRIO_LOW, (int)NULL);

  SYSCALL(CREATEPROCESS, (int)&gchild2state, PROCESS_PRIO_LOW, (int)NULL);

  SYSCALL(PASSEREN, (int)&sem_blkp8, 0, 0);
}

void child2() {
  print("child2 starts\n");

  int ppid = SYSCALL(GETPROCESSID, 1, 0, 0);
  if (ppid != p8pid) {
    print("Inconsistent (parent) process id for p8's first child\n");
    PANIC();
  }

  SYSCALL(CREATEPROCESS, (int)&gchild3state, PROCESS_PRIO_LOW, (int)NULL);

  SYSCALL(CREATEPROCESS, (int)&gchild4state, PROCESS_PRIO_LOW, (int)NULL);

  SYSCALL(PASSEREN, (int)&sem_blkp8, 0, 0);
}

/*p8leaf -- code for leaf processes*/

void p8leaf1() {
  print("leaf process (1) starts\n");
  SYSCALL(VERHOGEN, (int)&sem_endcreate[0], 0, 0);
  SYSCALL(PASSEREN, (int)&sem_blkp8, 0, 0);
}

void p8leaf2() {
  print("leaf process (2) starts\n");
  SYSCALL(VERHOGEN, (int)&sem_endcreate[1], 0, 0);
  SYSCALL(PASSEREN, (int)&sem_blkp8, 0, 0);
}

void p8leaf3() {
  print("leaf process (3) starts\n");
  SYSCALL(VERHOGEN, (int)&sem_endcreate[2], 0, 0);
  SYSCALL(PASSEREN, (int)&sem_blkp8, 0, 0);
}

void p8leaf4() {
  print("leaf process (4) starts\n");
  SYSCALL(VERHOGEN, (int)&sem_endcreate[3], 0, 0);
  SYSCALL(PASSEREN, (int)&sem_blkp8, 0, 0);
}

void p9() {
  print("p9 starts\n");
  SYSCALL(CREATEPROCESS, (int)&p10state, PROCESS_PRIO_LOW,
          (int)NULL); /* start p7		*/
  SYSCALL(PASSEREN, (int)&sem_blkp9, 0, 0);
}

void p10() {
  print("p10 starts\n");
  int ppid = SYSCALL(GETPROCESSID, 1, 0, 0);

  if (ppid != p9pid) {
    print("Inconsistent process id for p9!\n");
    PANIC();
  }

  SYSCALL(TERMPROCESS, ppid, 0, 0);

  print("Error: p10 didn't die with its parent!\n");
  PANIC();
}

void hp_p1() {
  print("hp_p1 starts\n");

  for (int i = 0; i < 100; i++) {
    SYSCALL(YIELD, 0, 0, 0);
  }

  SYSCALL(TERMPROCESS, 0, 0, 0);
  print("Error: hp_p1 didn't die!\n");
  PANIC();
}

void hp_p2() {
  print("hp_p2 starts\n");

  for (int i = 0; i < 10; i++) {
    SYSCALL(CLOCKWAIT, 0, 0, 0);
  }

  SYSCALL(TERMPROCESS, 0, 0, 0);
  print("Error: hp_p2 didn't die!\n");
  PANIC();
}
