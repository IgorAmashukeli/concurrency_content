#pragma once

#include <twist/ed/stdlike/atomic.hpp>
#include <twist/ed/wait/spin.hpp>

#include <cstdlib>

class TicketLock {
  using Ticket = uint64_t;

 public:
  // Do not change this method
  void Lock() {
    const Ticket this_thread_ticket = next_free_ticket_.fetch_add(1);

    twist::ed::SpinWait spin_wait;
    while (this_thread_ticket != owner_ticket_.load()) {
      spin_wait();
    }
  }

  /**
   if owner_ticket == next_free ticket => we change next_free ticket to
     owner_ticket + 1 == next_free ticket + 1, that means we say that next free
     ticket is now bigger, cause we acuired lock if owner_ticket !=
     next_free_ticket => we don't change next_free ticket and return false,
     cause the lock is already acquired The operation is, however, not atomic,
     but let's look, what can happen: if somebody does lock after load(),
     owner_ticket_value didn't change, but next_free_ticket change if somebody
     does unlock after load(), owner_ticket changed, and now it should be
     possible to enter, but it will not enter that means, mutual exclusion is
     not broken the only problem is absence of tylock liveliness, but this is
     not a mutex criteria of correctness it is only about lock-unlock liveliness
     correctness
     **/

  bool TryLock() {
    Ticket owner_ticket_value = owner_ticket_.load();
    return next_free_ticket_.compare_exchange_strong(owner_ticket_value,
                                                     owner_ticket_value + 1);
  }

  // Do not change this method
  void Unlock() {
    // Do we actually need atomic increment here?
    owner_ticket_.fetch_add(1);
  }

 private:
  twist::ed::stdlike::atomic<Ticket> next_free_ticket_{0};
  twist::ed::stdlike::atomic<Ticket> owner_ticket_{0};
};
