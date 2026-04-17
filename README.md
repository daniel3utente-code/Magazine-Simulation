# Multithreaded Warehouse Management System

This project implements a multithreaded warehouse management system in C, simulating a production line where items are generated, quality-checked, and logged asynchronously.The software utilizes POSIX threads (pthreads) and synchronization mechanisms to manage a shared circular buffer.

## System Overview
The system is divided into three specialized threads operating in parallel:

* **Producer Thread**: Creates a new product every 2 seconds, assigning it an incremental ID and a random quality value. It then inserts the piece into a shared buffer
* **Quality Control Thread**: Extracts products from the buffer and verifies their quality. If the quality is above 50, the product is sent to the log thread; otherwise, the memory is immediately freed.
* **Log Writer Thread**: Receives products that have passed the quality check and writes their data (ID and Quality) into the `magazzino.log` file in CSV format.

## Key Features
* **Synchronization**: Uses `pthread_mutex_t` for exclusive access to the buffer and `pthread_cond_t` (condition variables) to manage "buffer full" or "buffer empty" signals between threads.
* **Signal Handling**: Includes a handler for `SIGINT` (CTRL+C) that gracefully stops the threads via a broadcast on the condition variables, ensuring all resources are properly freed.
* **Circular Buffer**: Implements a FIFO-type buffer with a fixed size (`DIM_BUFFER = 10`) for communication between the producer and consumer.
* **Memory Management**: Uses dynamic allocation for products, guaranteeing that each item is removed from memory (free) once processed or upon program termination.

## Technical Requirements
* **Language**: C
* **Libraries**: `pthread.h`, `stdio.h`, `stdlib.h`, `signal.h`, `unistd.h`, `time.h`
* **Compiler**: GCC

## File Structure
* `main.c`: Contains the entry point, thread creation, and interruption signal management.
* `production.c`: Contains the operational logic of the three threads and the buffer management functions.
* `production.h`: Defines the `Product` structure and function prototypes.
* `magazzino.log`: The generated log file containing the production results.
