#include <mutex>
#include <wheels/test/framework.hpp>

// https://gitlab.com/Lipovsky/tinyfibers
#include <tf/sched/spawn.hpp>
#include <tf/sched/yield.hpp>
#include <tf/sync/mutex.hpp>

using tf::Mutex;
using tf::Spawn;
using tf::Yield;

/***

Task:
- Create a deadlock for one fiber (thread)


Idea:
- Create multiple locks with the same mutex, to create a deadlock.
Next lock can't be taken untill the first is unlocked.

**/

void OneFiberDeadLock() {
  Mutex mutex;

  auto fiber = [&] {
    // I am a Fiber
    std::lock_guard guard(mutex);
    std::lock_guard guard2(mutex);
  };

  Spawn(fiber).Join();

  // We do not expect to reach this line
  FAIL_TEST("No deadlock =(");
}
