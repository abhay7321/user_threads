<div align="center">
	<h1>multi-lib</h1>
   <h4>A multithreading library for Unix-like POSIX compliant systems</h4> 
</div>
<div align="center">
   <img align="center" width=20% src="assets/logonew.png" >
</div>

## Contents

- [About](#about)
- [Building](#building)
- [Mapping models](#mapping-models)
- [Available APIs](#available-apis)
- [Usage](#usage)
- [Running Tests](#running-tests)
- [Implementation Details](#implementation-details)
- [References](#references)

### About

`multi-lib` is a multithreading library that lets programs control multiple flows of work that run in an interleaved manner and supports user level threads in either a `One-One` mapping model or a `Many-One` mapping model. `multi-lib` is available for Unix-like POSIX conformant operating systems.

### Building

```sh
   # Run the root directory of the project
   make
   # To compile the binaries run
   make multi-lib

   # To compile the test suite
   make alltest
   # To run the tests
   ./run.sh

   # To generate API docs
   make docs

```

### Mapping models

A mapping model refers to the way in which a thread created by the user maps to the kernel level thread. There are three main types of which the two implemented in `multi-lib` are :

1. <b>One-one model</b>

   - One one model will use the `clone` system call to create a corresponding kernel thread for each user level thread. It is referred to as LWP.
   - LWPs share a same set of resources that are allocated to the process. These include address space, physical memory pages, file systems, open files and signal handlers.
   - Extensive resource sharing is the reason these processes are called light-weight processes.
   - One-one threading model is highly used in OS like Linux and Windows.

2. <b>Many-one model</b>

   - There is a mapping for many user level threads onto a single kernel thread.
   - The library handles the multiplexing of the threads into a single kernel thread with aspects such as scheduling, managing the context of threads etc.
   - The scheduling and context switching are entirely handled by the library and the kernel sees a single flow of execution.

   ```
                      User space                                            User space
         ┌─────────────────────────────────┐                   ┌─────────────────────────────────┐
         │                                 │                   │                                 │
         │                                 │                   │                                 │
         │                                 │                   │     ┌─┐  ┌─┐      ┌─┐ ┌─┐       │
         │                                 │                   │     └┼┘  └┼┘      └┼┘ └┼┘       │
         │                                 │                   │      │    │        │   │        │
         │     ┌─┐  ┌─┐      ┌─┐ ┌─┐       │                   │      │    │      ┌─┘   │        │
         │     └┼┘  └┼┘      └┼┘ └┼┘       │                   │      └───▲▼──────▼─◄───┘        │
         │      │    │        │   │        │                   │          │   map   │            │
         │      │    │        │   │        │                   │          │         │            │
         └──────┼────┼────────┼───┼────────┘                   └──────────┴────┬────┴────────────┘
                │    │        │   │                                            │
                │    │        │   │                                            │
         ┌──────┼────┼────────┼───┼────────┐                   ┌───────────────┼─────────────────┐
         │      │    │        │   │        │                   │               │                 │
         │      │    │        │   │        │                   │               │                 │
         │      │    │        │   │        │                   │               │                 │
         │      │    │       ┌┼┐ ┌┼┐       │                   │               │                 │
         │     ┌┼┐  ┌┼┐      └─┘ └─┘       │                   │              ┌┼┐                │
         │     └─┘  └─┘                    │                   │              └─┘                │
         │                                 │                   │                                 │
         │                                 │                   │                                 │
         │                                 │                   │                                 │
         └─────────────────────────────────┘                   └─────────────────────────────────┘
                     Kernel space                                          Kernel space


               One One Threading model                                Many One Threading model

   ```

## Available APIs

- `multi-lib` provides two mapping models and the desired model can be chosen without changing any API calls. The implementation remains abstract to the user programs.
- Each of the two models provides the following set of API calls:

   1. <b>Thread APIs</b>

      1. <b>Thread Creation</b>

         | Function          | Description                                |
         | ----------------- | ------------------------------------------ |
         | `thread_create()` | Creates a new thread                       |
         | `thread_join()`   | Waits for a callee thread to be terminated |
         | `thread_kill()`   | Send a signal to a specific thread         |
         | `thread_exit()`   | Exit the thread routine                    |

   2. <b>Synchronization Primitves APIs</b>

      1. <b>Spin Lock</b>

         | Function         	| Description                         |
         | ---------------------| ----------------------------------- |
         | `spinlock_init()`    | Initialize a spinlock object    |
         | `thread_lock()` 	| Acquire a spinlock                   |
         | `thread_unlock()` 	| Release a spinlock                 |

      2. <b>Mutex Lock</b>

         | Function          	   | Description                      |
         | ------------------------| -------------------------------- |
         | `mutexlock_init()`      | Initialize a mutex object        |
         | `thread_mutex_lock()`   | Acquire a mutex                  |
         | `thread_mutex_unlock()` | Release a mutex                  |

## Usage

To use multi-lib in your programs, do the following:


FOR using one-one mapping
```c

#include <stdio.h>
#include "one-one/thread.h"


int global_var = 0;
void func1(){
   printf("In thread routine 1");
   global_var++;
   return;
}

void func2(){
   printf("In thread routine 2");
   global_var++;
   return;
}

int main(){
   thread t1,t2;
   thread_create(&t1, func1, NULL , JOINABLE);
   thread_create(&t2, func2, NULL , JOINABLE);
   thread_join(t1,NULL);
   thread_join(t2,NULL);
   return 0;
}

```


FOR using many-one mapping
```c

#include <stdio.h>
#include "many-one/thread.h"


int global_var = 0;
void func1(){
   printf("In thread routine 1");
   global_var++;
   return;
}

void func2(){
   printf("In thread routine 2");
   global_var++;
   return;
}

int main(){
   thread t1,t2;
   thread_create(&t1, func1, NULL );
   thread_create(&t2, func2, NULL );
   thread_join(t1,NULL);
   thread_join(t2,NULL);
   return 0;
}

```

## Running Tests

* The library comes with an extensive test suite for checking the implementation and testing the performance of the library. 
* Each implementation has a set of unit tests that check the correctness of the APIs. 
* There is a test for checking the synchronization primitves and a classic program of readers writers to check the working of synchronization primitives namely mutex and spinlock. 
* The test suite also includes a robust testing program that checks for the error handling and incorrect input cases. 
* Finally there is a benchmark program which is a matrix multiplication program in the single and multi-threaded environments to compare the performance of using a threading library.


## Implementation Details
* To know the implementation details of one-one threading model, please check out it's [README](one-one/README.md)
* To know the implementation details of many-one threading model, please check out it's [README](many-one/README.md)


## References

- [Pthread Programming Ch.6](https://maxim.int.ru/bookshelf/PthreadsProgram/htm/r_47.html)
- [Introduction to pthreads](https://www.cs.cmu.edu/afs/cs/academic/class/15492-f07/www/pthreads.html)
- [Fundamentals of Multithreading](http://malgenomeproject.org/os2018fall/04_thread_b.pdf)
- [POSIX Linux Thread Library](https://www.cs.utexas.edu/~witchel/372/lectures/POSIX_Linux_Threading.pdf)
- [Implementing Threads](http://www.it.uu.se/education/course/homepage/os/vt18/module-4/implementing-threads/#kernel-level-threads)
- [POSIX Threads API](https://hpc-tutorials.llnl.gov/posix/)
- [Coroutines](http://www.csl.mtu.edu/cs4411.ck/common/Coroutines.pdf)
