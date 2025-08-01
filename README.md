# 🔄 Bounded Waiting Synchronization

Goals is to simulate concurrency involving **bounded waiting** and **thread synchronization**. It models a system where students seek help from a teaching assistant (TA) with a limited number of waiting spots (chairs) using threads, semaphores and mutexes in C.

---

## Overview

- Students alternate between working and seeking help
- A TA helps one student at a time and sleeps when no one is waiting
- A fixed number of chairs are available for students to wait
- If no chair is available then student goes back to working
- The system ensures proper coordination between threads and prevents race conditions

---

- Bounded waiting with semaphores (`sem_t`)
- Mutual exclusion using mutexes (`pthread_mutex_t`)
- Thread coordination using `pthread_create`, `pthread_join`
- Sleep/wake model using synchronization primitives

---

## 🛠️ How to Compile and Run

```bash
gcc -pthread -o bounded_sync bounded_sync.c
./bounded_sync
