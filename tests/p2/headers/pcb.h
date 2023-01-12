#ifndef PSB_H_INCLUDED
#define PSB_H_INCLUDED

#include "../generic_headers/pandos_const.h"
#include "../generic_headers/pandos_types.h"
#include "listx.h"

/* pcbFree list's functions */

/*
    Inizializza la lista pcbFree in modo da
    contenere tutti gli elementi della
    pcbFree_table. Questo metodo deve
    essere chiamato una volta sola in fase di
    inizializzazione della struttura dati.
*/
void initPcbs();

/*
    Inserisce il PCB puntato da p nella lista
    dei PCB liberi (pcbFree_h)
*/
void freePcb(pcb_t *p);

/*
    Restituisce NULL se la pcbFree_h è vuota. 
    Altrimenti rimuove un elemento dalla
    pcbFree, inizializza tutti i campi (NULL/0)
    e restituisce l’elemento rimosso.
*/
pcb_t *allocPcb();

/* Crea una lista di PCB, inizializzandola come lista vuota */
void mkEmptyProcQ(struct list_head *head);

/* Restituisce TRUE se la lista puntata da head è vuota, FALSE altrimenti. */
int emptyProcQ(struct list_head *head);

/* Inserisce l’elemento puntato da p nella coda dei processi puntata da head. */
void insertProcQ(struct list_head *head, pcb_t *p);

/*
    Restituisce l’elemento di testa della coda
    dei processi da head, SENZA
    RIMUOVERLO. Ritorna NULL se la coda
    non ha elementi.
*/
pcb_t *headProcQ(struct list_head *head);

/*
    Rimuove il primo elemento dalla coda dei
    processi puntata da head. Ritorna NULL se la
    coda è vuota. Altrimenti ritorna il puntatore 
    all’elemento rimosso dalla lista.
*/
pcb_t *removeProcQ(struct list_head *head);

/*
    Rimuove il PCB puntato da p dalla coda dei
    processi puntata da head. Se p non è presente 
    nella coda, restituisce NULL. (NOTA: p può 
    trovarsi in una posizione arbitraria della coda).
*/
pcb_t *outProcQ(struct list_head *head, pcb_t *p);

/* Pcb tree's functions */

/*
    Restituisce TRUE se il PCB puntato da p
    non ha figli, FALSE altrimenti.
*/
int emptyChild(pcb_t *p);

/*
    Inserisce il PCB puntato da p come figlio
    del PCB puntato da prnt.
*/
void insertChild(pcb_t *prnt, pcb_t *p);

/*
    Rimuove il primo figlio del PCB puntato
    da p. Se p non ha figli, restituisce NULL.
    Altrimenti ritorna il puntatore all'elemento rimosso
*/
pcb_t *removeChild(pcb_t *p);

/*
    Rimuove il PCB puntato da p dalla lista
    dei figli del padre. Se il PCB puntato da
    p non ha un padre, restituisce NULL,
    altrimenti restituisce l’elemento
    rimosso (cioè p). A differenza della 
    removeChild, p può trovarsi in una 
    posizione arbitraria (ossia non è 
    necessariamente il primo figlio del
    padre).
*/
pcb_t *outChild(pcb_t *p);

// funzione che, dato un id di un pcb, ti dice si trova nella lista dei pcb
// liberi
int isPcbFree(int pid);

#endif
