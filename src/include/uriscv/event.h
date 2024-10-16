// SPDX-FileCopyrightText: 2004 Mauro Morsiani
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef URISCV_EVENT_H
#define URISCV_EVENT_H

#include <boost/function.hpp>

#include "uriscv/types.h"

// Event class is used to keep track of the external events of the
// system: device operations and interrupt generation.
// Every object contains a device number saying what device will complete
// its operation and generate a interrupt, a TimeStamp saying when it
// will happen, and a pointer to the next event in a structure, if needed
// (set to NULL otherwise)

class Event {
public:
  typedef boost::function<void()> Callback;

  Event(uint64_t ts, Word inc, Callback callback);

  uint64_t getDeadline() const { return deadline; }
  Callback getCallback() const { return callback; }

  // This method links an Event to its successor in a structure
  void AddBefore(Event *ev);

  // This method inserts an Event after another, linking the former to the
  // successor of the latter
  void InsAfter(Event *ev);

  // This method returns the pointer to the successor of an Event
  Event *Next();

private:
  // Event verification time
  uint64_t deadline;

  // Event handler
  Callback callback;

  Event *next;
};

// This class implements a NULL-terminated sorted queue of Event objects,
// used to schedule the device events in the system.
// An object contains a pointer to the head of the queue, and a pointer to
// the last inserted Event (to speed up insertion).
// The queue is sorted on ascending timestamp order

class EventQueue {
public:
  // This method creates a new (empty) queue
  EventQueue();

  ~EventQueue();

  // This method returns TRUE if the queue is empty, FALSE otherwise
  bool IsEmpty() const { return head == NULL; }

  uint64_t nextDeadline() const;
  Event::Callback nextCallback() const;

  // This method creates a new Event object and inserts it in the
  // EventQueue; EventQueue is sorted on ascending time order
  uint64_t InsertQ(uint64_t tod, Word delay, Event::Callback callback);

  // This method removes the head of a (not empty) queue and sets it to the
  // following Event
  void RemoveHead();

private:
  // head of the queue
  Event *head;

  // last Event inserted
  Event *lastIns;
};

#endif // URISCV_EVENT_H
