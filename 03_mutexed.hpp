#pragma once

#include <mutex>
#include <twist/ed/stdlike/mutex.hpp>

//////////////////////////////////////////////////////////////////////

/*
 * Safe API for mutual exclusion
 *
 * Usage:
 *
 * Mutexed<std::vector<Apple>> apples;
 *
 * {
 *   auto owner_ref = apples->Acquire();
 *   owner_ref->push_back(Apple{});
 * }  // <- release ownership
 *
 */

/**
Task:

Mutexed class has one method: acquire.
Acquire returns the reference to the object, which can't be modified from now
on. Class guarantees, two threads can't acquire the same object Class
guarantees: after reference is destroyed, the mutex is unlocked.


**/

template <typename T, class Mutex = twist::ed::stdlike::mutex> class OwnerRef {
public:
  // constructor
  OwnerRef(T *object_ptr, Mutex &mutex)

      // creates object_ptr_
      // object_ptr_ will be destroyed after end of scope
      : object_ptr_(object_ptr),

        // guard: it will be destroyed after end of scope
        // destruction of guard <=> run of unlock function
        // mutex is unlocked after reference is destroyed in the end of scope
        guard_(mutex) {}

  // operator -> returns the pointer to the object to modify it, using OwnerRef
  T *operator->() { return object_ptr_; }

private:
  // pointer to the object
  T *object_ptr_;

  // lock_guard to the mutex
  std::lock_guard<Mutex> guard_;
};

// class mutexed
template <typename T, class Mutex = twist::ed::stdlike::mutex> class Mutexed {
public:
  // general template for some template arguments for a constructor of original
  // object
  template <typename... Args>

  // constructor is explicit (we shouldn't everytime construct Mutexed from Args
  // if they are in code)
  explicit Mutexed(Args &&...args)
      :

        // std::forward<Args>(args)... creates an object from args of type T
        // save this object in object_ filed
        object_(std::forward<Args>(args)...) {}

  // acquire
  OwnerRef<T, Mutex> Acquire() {
    // returns OwnerRef
    return OwnerRef(&object_, mutex_);
  }

private:
  // object
  T object_;

  // guard access to object_
  Mutex mutex_;
};

//////////////////////////////////////////////////////////////////////

template <typename T> auto Acquire(Mutexed<T> &object) {
  return object.Acquire();
}
