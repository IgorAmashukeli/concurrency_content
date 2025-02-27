#include <mutex>
#include <wheels/test/framework.hpp>

#include "../barrier.hpp"

// https://gitlab.com/Lipovsky/tinyfibers
#include <tf/sched/spawn.hpp>
#include <tf/sched/yield.hpp>
#include <tf/sync/mutex.hpp>
#include <tf/sync/wait_group.hpp>

/**

Task:

- Create deadlock with two fibers

Idea:

1) - create two mutexes, each one does a lock

2) - do lock inversion: one fiber does lock on mutex a first, second does lock
on mutex b first

3) first mutex a is acquired by fiber 1, yield

4) second mutex b is acquired by fiber 2, yield

5) there is no way to acquire mutex a or mutex b, because they are locked

6) therefore, there is no progress and nothing runs in fiber 1 or fiber 2

3) therefore we get a deadlock

 yeild is used to do the switching
**/

using tf::Mutex;
using tf::Spawn;
using tf::WaitGroup;
using tf::Yield;

void TwoFibersDeadLock() {
  // Mutexes
  Mutex a;
  Mutex b;

  // Fibers

  auto first = [&] {
    // I am a Fiber
    std::lock_guard guard_a(a);
    Yield();
    std::lock_guard guard_b(b);
  };

  auto second = [&] {
    // I am a Fiber
    std::lock_guard guard_b(b);
    Yield();
    std::lock_guard guard_a(a);
  };

  // No deadlock with one fiber

  // No deadlock expected here
  // Run routine twice to check that
  // routine leaves mutexes in unlocked state

  Spawn(first).Join();
  Spawn(first).Join();

  // Same for `second`
  Spawn(second).Join();
  Spawn(second).Join();

  ReadyToDeadLock();

  // Deadlock with two fibers
  WaitGroup wg;
  wg.Spawn(first).Spawn(second).Wait();

  // We do not expect to reach this line
  FAIL_TEST("No deadlock =(");
}
