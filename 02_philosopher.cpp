#include "philosopher.hpp"

#include <twist/test/inject_fault.hpp>

/**
Task:
Dining philosophers
Each philosopher has left and right fork.
Each philosopher in while true loop eats or think
When philosopher eats:
1) philosopher acquire forks
2) philosopher access plate
3) pholosopher release forks

Make philosophers eat without getting into the deadlock


idea: resourse ordering

change order of locking of one of the philosophers:

order of locking

first philosopher: 1 fork, then 2 fork
second philosopher: 2 fork, then 3 fork
...
pre-last philosopher: k - 1 fork, then k fork
last philosopher: 1 fork, then k fork

There is no circular wait: 1 resource waits for 2, 2 for 3, ... k-1 for k, k
for 1.

That means, there is no deadlock by definition.
**/

namespace dining {

Philosopher::Philosopher(Table &table, size_t seat)
    : table_(table), seat_(seat), left_fork_(table_.LeftFork(seat)),
      right_fork_(table_.RightFork(seat)) {}

void Philosopher::Eat() {
  AcquireForks();
  EatWithForks();
  ReleaseForks();
}

bool Philosopher::IsLast() const { return table_.ToRight(seat_) == 0; }

// Acquire left_fork_ and right_fork_
void Philosopher::AcquireForks() {
  // last philosopher changes the right and left order
  // to make correct order in circle
  if (IsLast()) {
    right_fork_.lock();
    left_fork_.lock();

    // other philosophers lock in right order
  } else {
    left_fork_.lock();
    right_fork_.lock();
  }
}

void Philosopher::EatWithForks() {
  table_.AccessPlate(seat_);
  // Try to provoke data race
  table_.AccessPlate(table_.ToRight(seat_));
  ++meals_;
}

// Release left_fork_ and right_fork_
void Philosopher::ReleaseForks() {
  // Your code goes here
  left_fork_.unlock();
  right_fork_.unlock();
}

void Philosopher::Think() {
  // Random pause or context switch
  twist::test::InjectFault();
}

} // namespace dining
