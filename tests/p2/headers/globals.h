#ifndef GLOBALS_H_INCLUDED
#define GLOBALS_H_INCLUDED

#include "./listx.h"
#include "./pcb.h"

/* Global Variables */
extern int activeProc; // Processi iniziati e non ancora terminati: attivi ||
                       // Process Count
extern int processId;  // Variabile globale utilizzata per assegnare un id unico
                       // ai processi creati
extern int blockedProc; // Processi 'blocked': in attesa di I/O oppure timer ||
                        // Soft-Block Count
extern struct list_head queueLowProc;  // Coda dei processi a bassa priorità
extern struct list_head queueHighProc; // Coda dei processi a alta priorità
extern pcb_PTR currentActiveProc;      // Puntatore processo in stato "running"
                                       // (attivo) || Current Process
extern cpu_t startTime; // Intero utilizzato per aggiornare il tempo di uso di
                        // un processo della cpu
extern pcb_PTR yieldHighProc; // Variabile booleana usata per fare yield su
                              // processi ad alta priorita'

// Vettore di interi per i semafori dei device|| Device Semaphores
/*
 * Consideriamo ogni coppia di semafori dei terminali come segue:
 * Primo semaforo dedicato alle operazioni di scrittura (transm)
 * Secondo semaforo dedicato alle operazioni di lettura (recv)
 */
extern int semIntervalTimer;
extern int semDiskDevice[8];
extern int semFlashDevice[8];
extern int semNetworkDevice[8];
extern int semPrinterDevice[8];
extern int semTerminalDeviceReading[8];
extern int semTerminalDeviceWriting[8];

/* Global Variables PHASE3  */

extern int master_sem; // master sem to controll the end of the uproc
extern support_t sd_table[UPROCMAX]; // table of usable support descriptor
extern struct list_head sd_free;     // list of free support descriptor
extern swap_t swap_pool_table[POOLSIZE];
extern int swap_pool_sem;

extern int semTermRead_phase3[8];
extern int semTermWrite_phase3[8];
extern int semPrinter_phase3[8];

#endif
