#include "headers/pcb.h"
static pcb_t pcbFree_table[MAXPROC]; /* PCB array with maximum size 'MAXPROC' */
static LIST_HEAD(pcbFree_h);         /* List of free PCBs                     */

extern void set_pid(pcb_PTR p);

void initPcbs() {
  for (int i = 0; i < MAXPROC; i++) {
    pcb_t *pcb = &pcbFree_table[i];
    list_add_tail(&pcb->p_list, &pcbFree_h);
  }
}

void freePcb(pcb_t *p) { list_add_tail(&p->p_list, &pcbFree_h); }

pcb_t *allocPcb() {
  if (list_empty(&pcbFree_h))
    return NULL;
  else {
    pcb_t *newElem = container_of(
        pcbFree_h.next, pcb_t, p_list); /* Getting first element of pcbFree  */
    list_del(pcbFree_h.next);           /* Delete first element from pcbFree */

    INIT_LIST_HEAD(&newElem->p_list); /* Initialize variables              */
    newElem->p_parent = NULL;
    INIT_LIST_HEAD(&newElem->p_child);
    INIT_LIST_HEAD(&newElem->p_sib);
    newElem->p_time = 0;
    newElem->p_semAdd = NULL;
    set_pid(newElem);

    newElem->p_supportStruct = NULL;

    return newElem;
  }
}

void mkEmptyProcQ(struct list_head *head) { INIT_LIST_HEAD(head); }

int emptyProcQ(struct list_head *head) { return list_empty(head); }

void insertProcQ(struct list_head *head, pcb_t *p) {
  list_add_tail(&p->p_list, head);
}

pcb_t *headProcQ(struct list_head *head) {
  return list_empty(head) ? NULL : container_of(head->next, pcb_t, p_list);
}

pcb_t *removeProcQ(struct list_head *head) {
  if (list_empty(head))
    return NULL;
  struct list_head *removedElement = head->next;
  list_del(head->next);
  return container_of(removedElement, pcb_t, p_list);
}

pcb_t *outProcQ(struct list_head *head, pcb_t *p) {
  struct list_head *pos;
  list_for_each(pos, head) {
    if (container_of(pos, pcb_t, p_list) == p) {
      list_del(&p->p_list);
      return p;
    }
  }
  return NULL;
}

int emptyChild(pcb_t *p) { return list_empty(&p->p_child); }

void insertChild(pcb_t *prnt, pcb_t *p) {
  list_add_tail(&p->p_sib,
                &prnt->p_child); /* Add p to prnt children list and sib list */
  p->p_parent = prnt;            /* prnt is the parent of p                  */
}

pcb_t *removeChild(pcb_t *p) {
  if (list_empty(&p->p_child))
    return NULL;
  pcb_t *firstChild = container_of(p->p_child.next, pcb_t, p_sib);

  list_del(
      p->p_child
          .next); /* Remove p's first child from p_child list and p_sib list */
  INIT_LIST_HEAD(
      &firstChild
           ->p_sib); /* Initialize firstChild p_sib list as a empty list */
  firstChild->p_parent = NULL; /* FirstChild as no parent anymore */

  return firstChild;
}

pcb_t *outChild(pcb_t *p) {
  pcb_t *prnt = p->p_parent;
  if (prnt != NULL) {

    list_del(&p->p_sib);       /* Remove p from p_child list and initialize */
    INIT_LIST_HEAD(&p->p_sib); /* firstChild p_sib list as a empty list     */

    p->p_parent = NULL; /* p as no parent anymore                    */

    return p;
  }
  return NULL;
}

int isPcbFree(int pid) {
  struct list_head *pos;
  list_for_each(pos, &pcbFree_h) {
    if (container_of(pos, pcb_t, p_list)->p_pid == pid)
      return TRUE;
  }
  return FALSE;
}
