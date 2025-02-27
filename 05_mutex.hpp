#pragma once

#include <cstdint>
#include <twist/ed/stdlike/atomic.hpp>
#include <twist/ed/wait/sys.hpp>

/**
 idea: three states:
 0 - lock is not acquired
 1 - lock is acquired, no data contention
 2 - lock is acquired, there is data contention

 No syscall in case of no data contention

 Futex is used for dealing with missed wakeup =>
 now WakeOne in Unlock() is only called when the kernel spinLock is acquired
 in futex syscall, this kernel spinlock can only be acquired after going to
 wait in Lock() function of mutex if there is one thread => locked_ = 1 and
 fetch_sub gives 1 => there is no syscall in case of no data contention

 No missed wake ups

 fetch_sub is used for dealing with missed wake up in case of data contention
 suppose we got locked_ value in Unlock and it was changed to 2 in Lock
 Lock goes to wait, while Unlock leaves and we get forever blocking
 To deal with this problem:
 if exchange(2) is done first:
   a) if wait starts and goes to system call, it will be waken
   b) if wait starts and wake wants to intervene, it can't, because of kernel
   spinlock c) if wait doesn't start, fetch_sub does locked_ = 1 => wait will
   be stopped and will leave its execution and then reenter the locked_, but
   at that point two situations can happen:
       1) locked_ == 0 => unlock was done => it will enter the critical
       section 2) locked_ == 1 => unlock wasn't done => it will get to the
       same situation recursively
           (you can prove everythin is ok by induction)
  if exchange(2) is not done first:
    a) fetch_sub(1) will make locked_ = 0 => the thread will enter the
    critical section, when there was unlock

 Mutual exclusion:

 If locked_ = 1 or 2 at first
 comparison, we will not pass if locked_ != 0 at second comparisopn, we will
 not pass. It means, we will only pass, if locked_ = 0. It could be done in
 unlock only. But what if unlock is not done, but there is another thread
 getting access, because locked_.fetch_sub(1) stored 0, and a thread could
 potentially enter and there will be no mutual exclusion? But in that case it
 means old value of locked_ was 1 => locked_.fetch_sub(1) returns 1 => there
 is nothing to do in unlock and unlock is actually done
 => therefore there is no such situation, when unlock is not done and somebody
 enters.
 It means, that there is mutual exclusion.

 Liveliness:
 Suppose, there is no liveliness.
 That means, all the threads are blocked in Lock,
 when there is nobody in critical section.
 That means, that after unlock, locked_ was changed to 0.
 At that time one thread should have enter critical section.
 If didn't happen, that means, that some thread changed locked_ to 1 or 2 and
 didn't enter exchange_ is atomic operation => that means that nobody can
 intervene in that time. That means, locked_ was either 0 or not 0 before
 exchange, and didn't change before the exchange_ was done
    1) if locked_ was 0 => this thread should enter critical section
    2) if locked_ was not 0 => somebody should have changed it.
      That means it was changed by a thread after unlock was done
      and locked_ was 0 and for this thread it two situation can happen
       => recursively somebody should have entered
 Therefore there is liveliness

 Summing it up, we have:
 mutual exclusion, no blocking and no missed wake ups, that means wait and
 wake work correctly
 Mutex guarantees global liveliness, because in absence of missed wake ups
 after unlock nobody is waiting forever => at least one thread can enter
 critical section 1) we have correct mutex (mutual exclusion +  global
 liveliness) 2) it is syscall-efficient: it uses futex only in data contention
 situation
**/

namespace stdlike {

class Mutex {
public:
  void Lock() {
    if (locked_.exchange(1) == 0) {
      return;
    }
    while (locked_.exchange(2) > 0) {
      twist::ed::futex::Wait(locked_, /*old=*/2);
    }
  }
  void Unlock() {
    if (locked_.fetch_sub(1) == 2) {
      auto wake_key = twist::ed::futex::PrepareWake(locked_);
      locked_.store(0);
      twist::ed::futex::WakeOne(wake_key);
    }
  }

private:
  twist::ed::stdlike::atomic<uint32_t> locked_{0};
}; // namespace stdlike
} // namespace stdlike
