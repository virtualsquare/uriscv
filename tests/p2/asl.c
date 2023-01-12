#include "headers/asl.h"

static semd_t semd_table[MAXPROC];     /* SEMD array with maximum size 'MAXPROC' */
static LIST_HEAD(semdFree_h);          /* List of free SEMD                      */
static LIST_HEAD(ASL_h);               /* Active Semaphore List                  */


/*
    Given a semAdd key, return the pointer to the Semaphore
    with that key if present in ASL, else return NULL
*/
semd_PTR findASL(int *semAdd) {
    semd_PTR sem;
    struct list_head *pos;

    list_for_each(pos,&ASL_h) {                     /* Looking for Semaphore */
        sem = container_of(pos, semd_t, s_link);
        if(sem->s_key == semAdd)
            return sem;
    }
    return NULL;
}


/* Funzione che cerca un pcb bloccato dato il pid nella lista delle dei semafori attivi  */
pcb_PTR isPcbBlocked(int pid) {
    semd_PTR sem;
    struct list_head *pos;
    pcb_PTR p;
    list_for_each(pos, &ASL_h) {                                /* Looking for Semaphore */
        sem = container_of(pos, semd_t, s_link);
        struct list_head *tmp;
        list_for_each(tmp, &sem->s_procq) {                     /* Looking for Pcb */
            if((p = container_of(tmp, pcb_t, p_list))->p_pid == pid)
                return p;
        }
    }
    return NULL;
}


/*
    Delete the Semaphore from ASL if it has no pcb blocked anymore
    and insert the Semaphore in semdFree list.
    Return TRUE if the Semaphore is freed, else FALSE.
*/
static int isSemdFree(semd_PTR sem) {
    if(list_empty(&sem->s_procq)) {
        list_del(&sem->s_link);
        list_add_tail(&sem->s_link, &semdFree_h);
        return TRUE;
    }
    return FALSE;
}


int insertBlocked(int *semAdd, pcb_t *p) {
    semd_PTR sem = findASL(semAdd);

    if(sem != NULL) {                                         /*If p found add p to blocked process queue   */
        p->p_semAdd = semAdd;
        (*sem->s_key) = 0;
        insertProcQ(&sem->s_procq, p);
    } else {
        if(list_empty(&semdFree_h))
            return TRUE;                                      /* If there are no free Semaphore return TRUE */

        sem = container_of(semdFree_h.next, semd_t, s_link);  /* Get the first free Semaphore               */
        list_del(&sem->s_link);                               /* Remove the first Semaphore from semdFree   */
        
        /* Initialize variables */
        p->p_semAdd = semAdd;
        sem->s_key  = semAdd;
        INIT_LIST_HEAD(&sem->s_procq);
        insertProcQ(&sem->s_procq, p);                         /* Insert blocked process                    */
        list_add_tail(&sem->s_link, &ASL_h);                   /* Add Semaphore to the active ones list     */
    }
    return FALSE;
}


pcb_t* removeBlocked(int *semAdd) {
    semd_PTR sem = findASL(semAdd);
    pcb_PTR ret;

    if (sem != NULL && !isSemdFree(sem)) {
        ret = container_of(sem->s_procq.next, pcb_t, p_list);  /* Get the first PCB */
        list_del(sem->s_procq.next);
        ret->p_semAdd = NULL;
        isSemdFree(sem);
        return ret;
    }
    return NULL;
}


pcb_t* outBlocked(pcb_t *p) {
    semd_PTR sem;
    pcb_PTR pList;
    struct list_head *pos;

    if(p->p_semAdd != NULL && (sem = findASL(p->p_semAdd)) != NULL) {
        list_for_each(pos, &sem->s_procq) {                           /* Looking for a blocked pcb */
            pList = container_of(pos, pcb_t, p_list);
            if(p == pList) {
                p->p_semAdd = NULL;
                list_del(&p->p_list);
                isSemdFree(sem);
                return p;
            }
        }
    }
    return NULL;
}


pcb_t* headBlocked(int *semAdd) {
    semd_PTR sem = findASL(semAdd);
    
    if(sem != NULL && !isSemdFree(sem))
        return container_of(sem->s_procq.next, pcb_t, p_list);
    
    return NULL;
}

//inizializziamo i semafori come liberi
void initASL() {
    for(int i=0; i < MAXPROC; i++) {
		semd_t* semd = &semd_table[i];
		list_add_tail(&semd->s_link, &semdFree_h);
	}
}
