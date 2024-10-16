// SPDX-FileCopyrightText: 2004 Mauro Morsiani
//
// SPDX-License-Identifier: GPL-3.0-or-later

/****************************************************************************
 *
 * This module implements some classes used from the SystemBus for the
 * scheduling of device events (such as device operations completion and
 * interrupts generation).  They are: Event class, to keep track of single
 * events, as required by devices; and EventQueue class, to organize the
 * Events into a time-ordered queue.
 *
 ***************************************************************************/

#include "uriscv/event.h"

#include <cassert>
#include <stdio.h>
#include <stdlib.h>

#include "uriscv/const.h"
#include "uriscv/time_stamp.h"
#include "uriscv/utility.h"

Event::Event(uint64_t ts, Word inc, Callback callback)
    : deadline(ts + inc), callback(callback), next(NULL) {}

// This method links an Event to its successor in a structure
void Event::AddBefore(Event *ev) { next = ev; }

// This method inserts an Event after another, linking the former to the
// successor of the latter
void Event::InsAfter(Event *ev) {
  next = ev->next;
  ev->next = this;
}

// This method returns the pointer to the successor of an Event
Event *Event::Next() { return (next); }

// This method creates a new (empty) queue
EventQueue::EventQueue() {
  head = NULL;
  lastIns = NULL;
}

// This method deletes the queue and its associated structures
EventQueue::~EventQueue() {
  Event *p, *q;

  p = head;
  q = NULL;
  while (p != NULL) {
    q = p->Next();
    delete p;
    p = q;
  }
}

uint64_t EventQueue::nextDeadline() const {
  assert(!IsEmpty());
  return head->getDeadline();
}

Event::Callback EventQueue::nextCallback() const {
  assert(!IsEmpty());
  return head->getCallback();
}

// This method creates a new Event object and inserts it in the
// EventQueue; EventQueue is sorted on ascending time order

uint64_t EventQueue::InsertQ(uint64_t tod, Word delay,
                             Event::Callback callback) {
  Event *ins, *p, *q;

  ins = new Event(tod, delay, callback);
  if (IsEmpty()) {
    head = ins;
  } else if (ins->getDeadline() <= head->getDeadline()) {
    // "ins" has to happen before that at the head of the queue;
    // should be put before it
    ins->AddBefore(head);
    head = ins;
  } else {
    // should find place in queue: check lastIns to shorten search time
    if (lastIns != NULL && !(ins->getDeadline() <= lastIns->getDeadline()))
      // can start from lastIns
      p = lastIns;
    else
      // must start from the head
      p = head;

    q = p;
    while (p != NULL && p->getDeadline() <= ins->getDeadline()) {
      q = p;
      p = p->Next();
    }
    // place found: insert after q and before p
    ins->InsAfter(q);
  }
  lastIns = ins;
  return ins->getDeadline();
}

// This method removes the head of a (not empty) queue and sets it to the
// following Event
void EventQueue::RemoveHead() {
  Event *p;

  if (!IsEmpty()) {
    p = head;
    head = head->Next();

    if (p == lastIns)
      // reposition lastIns to the new head
      lastIns = head;

    delete p;
  }
}
